#include <stdio.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#include "Mini2.h"

#define TAG "MAIN"

// ---------------- GPIO ----------------
#define UART_TX GPIO_NUM_1
#define UART_RX GPIO_NUM_2

#define POTI_PIN GPIO_NUM_4

#define MULTI_BTN GPIO_NUM_10
#define ZOOM_BTN GPIO_NUM_9
#define FLASH_BTN GPIO_NUM_7
#define PALETTE_BTN GPIO_NUM_8

// ---------------- CONFIG ----------------
#define BUTTON_DEBOUNCE_US 20000
#define DOUBLE_CLICK_WINDOW 280000

//----------------PRESETS------------------
#define PRESET_COUNT 4

value_preset_t presets[PRESET_COUNT] = {
    // 0: NAVIGATION (clean, natural)
    { true, WHOT, LinearStretch, 50, 50, 0, 40, true, false, false },
    // 1: DETECTION (targets pop hard)
    { true, WHOT, Highlight, 50, 80, 1, 60, true, false, false },
    // 2: DETAIL (see textures + surfaces)
    { true, WHOT, HighContrast, 50, 70, 0, 70, true, false, false },
    // 3: OUTLINE (edge detection)
    { true, WHOT, Outline, 50, 50, 1, 50, true, false, false }
};

int current_preset = 0;

// ---------------- CAMERA ----------------
Mini2_t cam = {
    .uart_port = UART_NUM_1,
    .uart_tx = UART_TX,
    .uart_rx = UART_RX,
    .variant = Mini2_256
};

// ---------------- STATE ----------------
bool palette_state = false;

int zoom_index = 0;
int zoom_steps[3] = {10, 40, 80};
int zoom_level = 10;

// --- CONTROL STATE ---
int brightness = 50;
int contrast = 50;

bool adjusting_contrast = false;

int brightness_adc_anchor = 0;
int contrast_adc_anchor = 0;

bool brightness_active = false;
bool contrast_active = false;

// ---------------- BUTTON ----------------
bool button_state = false;
bool last_button_state = false;

int64_t press_time = 0;
bool nuc_triggered = false;

bool click_pending = false;
int click_count = 0;
int64_t last_click_time = 0;

int64_t last_button_change = 0;

bool flash_btn_last = false;
bool flash_toggle = false; // false = 3 flashes, true = 5 flashes

// ---------------- HELPERS ----------------
void restore_crosshair()
{
    Mini2_set_crosshair(&cam, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    Mini2_set_crosshair(&cam, true);
}

void flash_crosshair(int count, int delay_ms)
{
    for (int i = 0; i < count; i++)
    {
        Mini2_set_crosshair(&cam, false);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));

        Mini2_set_crosshair(&cam, true);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

//-------------PRESET HELPERS----------------
void apply_preset(value_preset_t *p)
{
    Mini2_set_scene_mode(&cam, p->scene_mode);
    Mini2_set_contrast(&cam, p->contrast);
    Mini2_set_edge_enhancment(&cam, p->edge_enhancment_gear);
    Mini2_set_detail_enhancement(&cam, p->detail_enhancement_gear);
    Mini2_set_burn_protection(&cam, p->burn_protection_en);
    Mini2_set_auto_shutter(&cam, p->auto_shutter_en);

    ESP_LOGI(TAG, "Preset applied");
}

// ---------------- MAIN LOOP ----------------
void loop_task(void *pvParameters)
{
    adc_unit_t unit;
    adc_channel_t channel;
    adc_oneshot_io_to_channel(POTI_PIN, &unit, &channel);

    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, channel, &config);

    int adc_raw;
    int last_adc_val = 0;

    static bool last_zoom_btn = false;
    static bool last_palette_btn = false;
    static bool last_adjusting_contrast = false;

    while (1)
    {
        int64_t now = esp_timer_get_time();

        // -------- ZOOM BUTTON --------
        bool zoom_btn = (gpio_get_level(ZOOM_BTN) == 0);

        if (zoom_btn && !last_zoom_btn)
        {
            zoom_index = (zoom_index + 1) % 3;
            zoom_level = zoom_steps[zoom_index];

            Mini2_set_centre_zoom(&cam, zoom_level);

            ESP_LOGI(TAG, "Zoom %.1fx", zoom_level / 10.0);

            if (zoom_level == 10)
                Mini2_set_crosshair(&cam, true);
            else
                Mini2_set_crosshair(&cam, false);
        }

        last_zoom_btn = zoom_btn;

        // -------- PALETTE BUTTON --------
        bool palette_btn = (gpio_get_level(PALETTE_BTN) == 0);

        if (palette_btn && !last_palette_btn)
        {
            palette_state = !palette_state;

            Mini2_set_color_pallet(&cam, palette_state ? BHOT : WHOT);
            restore_crosshair();

            ESP_LOGI(TAG, "Palette %s", palette_state ? "BHOT" : "WHOT");
        }

        last_palette_btn = palette_btn;

        // -------- FLASH BUTTON --------   <-- 🔥 PASTE IT RIGHT HERE
        bool flash_btn = (gpio_get_level(FLASH_BTN) == 0);

        if (flash_btn && !flash_btn_last)
        {
            if (!flash_toggle)
            {
                flash_crosshair(3, 120);
                ESP_LOGI(TAG, "Flash: 3x");
            }
            else
            {
                flash_crosshair(5, 60);
                ESP_LOGI(TAG, "Flash: 5x fast");
            }

            flash_toggle = !flash_toggle;
        }

        flash_btn_last = flash_btn;

        // -------- MULTI BUTTON --------
        bool raw_button = (gpio_get_level(MULTI_BTN) == 0);

        if (raw_button != last_button_state)
            last_button_change = now;

        if (now - last_button_change > BUTTON_DEBOUNCE_US)
        {
            if (raw_button != button_state)
            {
                button_state = raw_button;

                if (button_state)
                {
                    press_time = now;
                    nuc_triggered = false;
                }
                else
                {
                    if (!nuc_triggered && (now - press_time < 600000))
                        click_pending = true;
                }
            }
        }

        // -------- MODE SWITCH --------
        adjusting_contrast = button_state;

        if (adjusting_contrast != last_adjusting_contrast)
        {
            // HARD RESET (prevents snap)
            brightness_active = false;
            contrast_active = false;

            brightness_adc_anchor = last_adc_val;
            contrast_adc_anchor = last_adc_val;

            ESP_LOGI(TAG, "Mode: %s", adjusting_contrast ? "CONTRAST" : "BRIGHTNESS");
        }

        last_adjusting_contrast = adjusting_contrast;

        // -------- CLICK LOGIC --------
        if (click_pending)
        {
            click_pending = false;
            click_count++;
            last_click_time = now;
        }

        if (click_count > 0 && (now - last_click_time) > DOUBLE_CLICK_WINDOW)
        {
            if (click_count == 1)
            {
                current_preset = (current_preset + 1) % PRESET_COUNT;
                apply_preset(&presets[current_preset]);
                ESP_LOGI(TAG, "Preset %d", current_preset);
            }
            else if (click_count == 2)
            {
                palette_state = !palette_state;
                Mini2_set_color_pallet(&cam, palette_state ? BHOT : WHOT);
                restore_crosshair();
            }

            click_count = 0;
        }

        // Long press → NUC
        if (button_state && !nuc_triggered && (now - press_time > 3500000))
        {
            Mini2_NUC(&cam);
            ESP_LOGI(TAG, "NUC");
            nuc_triggered = true;
        }

        last_button_state = raw_button;

        // -------- ADC --------
        adc_oneshot_read(adc_handle, channel, &adc_raw);

        // light smoothing
        adc_raw = (adc_raw * 2 + last_adc_val) / 3;

        bool moved = abs(adc_raw - last_adc_val) > 8;

        // ---------- CONTRAST ----------
        if (adjusting_contrast)
        {
            if (!contrast_active)
            {
                if (abs(adc_raw - contrast_adc_anchor) > 20)
                {
                    contrast_active = true;
                    ESP_LOGI(TAG, "Contrast engaged");
                }
            }

            if (contrast_active && moved)
            {
                // flipped direction = more intuitive
                contrast = 100 - ((adc_raw * 100) / 4095);

                Mini2_set_contrast(&cam, contrast);
                ESP_LOGI(TAG, "Contrast %d", contrast);
            }
        }
        // ---------- BRIGHTNESS ----------
        else
        {
            if (!brightness_active)
            {
                if (abs(adc_raw - brightness_adc_anchor) > 30)
                {
                    brightness_active = true;
                    ESP_LOGI(TAG, "Brightness engaged");
                }
            }

            if (brightness_active && moved)
            {
                brightness = (adc_raw * 100) / 4095;

                Mini2_set_brightness(&cam, brightness);
                ESP_LOGI(TAG, "Brightness %d", brightness);
            }
        }

        last_adc_val = adc_raw;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ---------------- APP MAIN ----------------
void app_main(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << MULTI_BTN) |
                        (1ULL << ZOOM_BTN) |
                        (1ULL << PALETTE_BTN) |
                        (1ULL << FLASH_BTN),
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    Mini2_init(&cam);
    vTaskDelay(pdMS_TO_TICKS(2500)); //minimum delay time...
    apply_preset(&presets[current_preset]);

    Mini2_set_color_pallet(&cam, WHOT);
    Mini2_set_crosshair(&cam, true);
    Mini2_set_centre_zoom(&cam, 10);
    Mini2_set_brightness(&cam, brightness);

    xTaskCreate(loop_task, "loop", 8192, NULL, 5, NULL);
}
