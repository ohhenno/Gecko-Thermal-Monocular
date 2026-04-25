#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"

#include "driver/gpio.h"

#include "Mini2.h"

#include "esp_adc/adc_oneshot.h"

// ---------------- GPIO ----------------
#define UART_TX GPIO_NUM_1
#define UART_RX GPIO_NUM_2

#define MULTI_BTN GPIO_NUM_10
#define ZOOM_BTN GPIO_NUM_9
#define FLASH_BTN GPIO_NUM_7

#define BRIGHT_UP GPIO_NUM_5
#define BRIGHT_DOWN GPIO_NUM_6

// ---------------- TIMING ----------------
#define CLICK_MAX_US        250000
#define DOUBLE_CLICK_US     350000

// ---------------- CAMERA ----------------
Mini2_t cam = {
    .uart_port = UART_NUM_1,
    .uart_tx = UART_TX,
    .uart_rx = UART_RX,
    .variant = Mini2_256
};

// ---------------- STATE ----------------
int zoom_steps[3] = {10, 20, 60};
int zoom_index = 0;

bool palette_state = false;

// ---------------- ZOOM ----------------
bool last_zoom_btn = false;
bool zoom_hold_active = false;
bool zoom_double_wait = false;

int64_t zoom_press_time = 0;
int64_t last_release_time = 0;
int64_t last_repeat_time = 0;

// ---------------- MULTI ----------------
bool last_multi = false;
int64_t multi_press_time = 0;
int64_t last_multi_click = 0;
bool multi_hold_triggered = false;

// ---------------- FLASH ----------------
bool last_flash = false;
bool flash_mode = false;

//------------ADC---------------
#define POTI_PIN GPIO_NUM_4

int brightness = 60;
int contrast = 50;
// ---------------- PRESETS ----------------
#define PRESET_COUNT 3
int current_mode = 0;

value_preset_t presets[PRESET_COUNT] = {
    {
        .preset_en = true,
        .pseudo_color = WHOT,
        .scene_mode = Highlight,
        .contrast = 100,
        .edge_enhancment_gear = 1,
        .detail_enhancement_gear = 55,
        .burn_protection_en = true,
        .auto_shutter_en = true,
        .breathing = false
    },
    {
        .preset_en = true,
        .pseudo_color = WHOT,
        .scene_mode = HighContrast,
        .contrast = 100,
        .edge_enhancment_gear = 1,
        .detail_enhancement_gear = 55,
        .burn_protection_en = true,
        .auto_shutter_en = true,
        .breathing = false
    },
    {
        .preset_en = true,
        .pseudo_color = WHOT,
        .scene_mode = Outline,
        .contrast = 100,
        .edge_enhancment_gear = 1,
        .detail_enhancement_gear = 50,
        .burn_protection_en = true,
        .auto_shutter_en = true,
        .breathing = false
    }
};

// ---------------- HELPERS ----------------
static inline void pulse_gpio(gpio_num_t pin, int ms)
{
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(pin, 1);
}

void run_flash_pattern()
{
    if (!flash_mode)
    {
        for (int i = 0; i < 3; i++)
        {
            Mini2_set_crosshair(&cam, false);
            vTaskDelay(pdMS_TO_TICKS(120));
            Mini2_set_crosshair(&cam, true);
            vTaskDelay(pdMS_TO_TICKS(180));
        }
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            Mini2_set_crosshair(&cam, false);
            vTaskDelay(pdMS_TO_TICKS(40));
            Mini2_set_crosshair(&cam, true);
            vTaskDelay(pdMS_TO_TICKS(60));
        }
    }
    flash_mode = !flash_mode;
}

// 🔧 SAFE PRESET APPLY (ONLY VALID FUNCTIONS)
void apply_preset(int mode)
{
    value_preset_t *p = &presets[mode];

    if (!p->preset_en) return;

    // Scene mode (this is the BIG one)
    Mini2_set_scene_mode(&cam, p->scene_mode);

    // Contrast
    Mini2_set_contrast(&cam, p->contrast);

    // Edge enhancement (your driver spelling)
    Mini2_set_edge_enhancment(&cam, p->edge_enhancment_gear);

    // Detail enhancement
    Mini2_set_detail_enhancement(&cam, p->detail_enhancement_gear);

    // Burn protection
    Mini2_set_burn_protection(&cam, p->burn_protection_en);

    // Auto shutter / NUC behavior
    Mini2_set_auto_shutter(&cam, p->auto_shutter_en);

    // IMPORTANT: Palette stays user-controlled
    Mini2_set_color_pallet(&cam, palette_state ? BHOT : WHOT);
}

// ---------------- LOOP ----------------
void loop_task(void *pv)
{
    int64_t last_crosshair = 0;

    adc_unit_t unit;
    adc_channel_t ch;
    adc_oneshot_io_to_channel(POTI_PIN, &unit, &ch);

    adc_oneshot_unit_handle_t adc;
    adc_oneshot_unit_init_cfg_t cfg = {.unit_id = unit};
    adc_oneshot_new_unit(&cfg, &adc);

    adc_oneshot_chan_cfg_t ch_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12};
    adc_oneshot_config_channel(adc, ch, &ch_cfg);

    int last_adc = 0;

    while (1)
    {
        int64_t now = esp_timer_get_time();

        bool zoom = !gpio_get_level(ZOOM_BTN);
        bool multi = !gpio_get_level(MULTI_BTN);
        bool flash = !gpio_get_level(FLASH_BTN);

        // ================= ZOOM =================
        if (zoom && !last_zoom_btn)
        {
            zoom_press_time = now;
            zoom_hold_active = false;

            zoom_index = (zoom_index + 1) % 3;
            int zoom_val = zoom_steps[zoom_index];

            Mini2_set_centre_zoom(&cam, zoom_val);

            if (zoom_val == 10)
                Mini2_set_crosshair(&cam, true);
            else
                Mini2_set_crosshair(&cam, false);
        }

        if (!zoom && last_zoom_btn)
        {
            int64_t dt = now - zoom_press_time;

            if (dt < 250000)
            {
                if (zoom_double_wait && (now - last_release_time < 400000))
                {
                    pulse_gpio(BRIGHT_DOWN, 60);
                    zoom_double_wait = false;
                }
                else
                {
                    zoom_double_wait = true;
                    last_release_time = now;
                }
            }

            zoom_hold_active = false;
        }

        if (zoom)
        {
            int64_t held = now - zoom_press_time;

            if (!zoom_hold_active && held > 1000000)
            {
                pulse_gpio(BRIGHT_UP, 60);
                zoom_hold_active = true;
                last_repeat_time = now;
            }

            if (zoom_hold_active && (now - last_repeat_time > 1000000))
            {
                pulse_gpio(BRIGHT_UP, 60);
                last_repeat_time = now;
            }
        }

        if (zoom_double_wait && (now - last_release_time > 500000))
            zoom_double_wait = false;

        last_zoom_btn = zoom;

        // ================= MULTI =================
        if (multi && !last_multi)
            {
                multi_press_time = now;
            }

            if (!multi && last_multi)
            {
                int64_t dt = now - multi_press_time;

                if (dt < CLICK_MAX_US)
                {
                    if ((now - last_multi_click) < DOUBLE_CLICK_US)
                    {
                        // DOUBLE CLICK → PALETTE
                        palette_state = !palette_state;
                        Mini2_set_color_pallet(&cam, palette_state ? BHOT : WHOT);
                        last_multi_click = 0;
                    }
                    else
                    {
                        last_multi_click = now;
                    }
                }
            }

            // HOLD → NUC (ONE SHOT)
            if (multi && !multi_hold_triggered && (now - multi_press_time > 3000000))
            {
                Mini2_NUC(&cam);
                multi_hold_triggered = true;
            }

            // SINGLE CLICK
            if (last_multi_click && (now - last_multi_click > DOUBLE_CLICK_US))
            {
                current_mode = (current_mode + 1) % PRESET_COUNT;
                apply_preset(current_mode);
                last_multi_click = 0;
            }

            // 🔥 RELEASE DETECT (correct place)
            if (!multi && last_multi)
            {
                multi_hold_triggered = false;
            }

            // 🔥 UPDATE STATE LAST

            last_multi = multi;

        // ================= FLASH =================
        if (flash && !last_flash)
        {
            run_flash_pattern();
        }
        last_flash = flash;

        // ================= CROSSHAIR KEEP ALIVE =================
        if (zoom_steps[zoom_index] == 10 && (now - last_crosshair > 200000))
        {
            Mini2_set_crosshair(&cam, true);
            last_crosshair = now;
        }

        // ================= ADC =================
        int adc_raw;
        adc_oneshot_read(adc, ch, &adc_raw);

        // smooth
        adc_raw = (adc_raw * 2 + last_adc) / 3;

        if (abs(adc_raw - last_adc) > 10)
        {
            if (multi)
            {
                // MULTI HELD → CONTRAST
                contrast = 20 + (adc_raw * 80) / 4095;
                Mini2_set_contrast(&cam, contrast);
            }
            else
            {
                // NORMAL → BRIGHTNESS (Mini2 side, not OLED)
                brightness = 20 + (adc_raw * 80) / 4095;
                Mini2_set_brightness(&cam, brightness);
            }
        }

            last_adc = adc_raw;

        vTaskDelay(pdMS_TO_TICKS(8));
    }
}

// ---------------- MAIN ----------------
void app_main(void)
{
    gpio_config_t in = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =
            (1ULL << ZOOM_BTN) |
            (1ULL << MULTI_BTN) |
            (1ULL << FLASH_BTN),
        .pull_up_en = GPIO_PULLUP_ENABLE};
    gpio_config(&in);

    gpio_config_t out = {
        .pin_bit_mask = (1ULL << BRIGHT_UP) | (1ULL << BRIGHT_DOWN),
        .mode = GPIO_MODE_OUTPUT_OD};
    gpio_config(&out);

    gpio_set_level(BRIGHT_UP, 1);
    gpio_set_level(BRIGHT_DOWN, 1);

    Mini2_init(&cam);
    vTaskDelay(pdMS_TO_TICKS(2000));

    Mini2_set_color_pallet(&cam, WHOT);
    Mini2_set_centre_zoom(&cam, 10);
    Mini2_set_crosshair(&cam, true);

    xTaskCreate(loop_task, "loop", 8192, NULL, 5, NULL);
}
