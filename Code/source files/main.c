#include <stdio.h>
#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_adc/adc_oneshot.h"

#define TAG "TAC"

// ---------------- GPIO ----------------
#define UART_PORT UART_NUM_1
#define UART_TX GPIO_NUM_1
#define UART_RX GPIO_NUM_2

#define POTI_PIN GPIO_NUM_4

#define MULTI_BTN GPIO_NUM_10
#define ZOOM_BTN GPIO_NUM_9
#define FLASH_BTN GPIO_NUM_7
#define PALETTE_BTN GPIO_NUM_8

bool recording = false;

// ---------------- CONFIG ----------------
#define BUTTON_DEBOUNCE_US 20000

// ---------------- STATE ----------------
bool palette_state = false;

int palette_index = 0;
uint8_t palette_modes[6] = {0, 1, 2, 3, 4, 5};

int zoom_index = 0;
int zoom_steps[3] = {10, 20, 40};
int zoom_level = 10;

int brightness = 64;
int contrast = 64;

int last_adc_val = 0;

// ---------------- BUTTON STATE ----------------
bool last_zoom_btn = false;
bool last_palette_btn = false;
bool last_flash_btn = false;
bool last_multi_btn = false;

int64_t press_time = 0;
bool nuc_triggered = false;

// ---------------- UART ----------------
void uart_init()
{
    uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &config);
    uart_set_pin(UART_PORT, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

uint8_t calc_checksum(uint8_t *buf, int len)
{
    uint16_t sum = 0;
    for (int i = 0; i < len; i++)
        sum += buf[i];
    return sum & 0xFF;
}

void p6_send(uint8_t cmd1, uint8_t cmd2, uint8_t *data)
{
    uint8_t pkt[13] = {0};

    pkt[0] = 0x55;
    pkt[1] = cmd1;
    pkt[2] = cmd2;

    memcpy(&pkt[3], data, 8);

    pkt[11] = calc_checksum(pkt, 11);
    pkt[12] = 0xAA;

    uart_write_bytes(UART_PORT, (const char *)pkt, 13);
}

// ---------------- COMMANDS ----------------
void P6_set_brightness(uint8_t val)
{
    uint8_t d[8] = {0};
    d[0] = val;
    p6_send(0x2A, 0x01, d);
}

void P6_set_contrast(uint8_t val)
{
    uint8_t d[8] = {0};
    d[0] = val;
    p6_send(0x2A, 0x02, d);
}

void P6_set_denoise(uint8_t val)
{
    uint8_t d[8] = {0};
    d[0] = val;
    p6_send(0x2A, 0x04, d);
}

void P6_set_enhancement(uint8_t val)
{
    uint8_t d[8] = {0};
    d[0] = val;
    p6_send(0x2A, 0x03, d);
}

void P6_set_palette_index(int index)
{
    uint8_t d[8] = {0};
    p6_send(0x2D, palette_modes[index], d);
}

void P6_set_palette(bool black_hot)
{
    uint8_t d[8] = {0};
    p6_send(0x2D, black_hot ? 1 : 0, d);
}

void P6_set_zoom(uint8_t zoom)
{
    uint8_t d[8] = {0};
    d[0] = zoom;
    p6_send(0x2B, 0x00, d);
}

void P6_nuc()
{
    uint8_t d[8] = {0};
    p6_send(0x26, 0x02, d);
}

void P6_set_auto_nuc(uint32_t ms)
{
    uint8_t d[8] = {0};

    d[0] = ms & 0xFF;
    d[1] = (ms >> 8) & 0xFF;
    d[2] = (ms >> 16) & 0xFF;
    d[3] = (ms >> 24) & 0xFF;

    p6_send(0x26, 0x04, d);
}

// flash feedback
void flash_feedback(int count, int delay_ms)
{
    for (int i = 0; i < count; i++)
    {
        P6_set_palette(true);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        P6_set_palette(false);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

// ---------------- LOOP ----------------
void loop_task(void *pvParameters)
{
    adc_unit_t unit;
    adc_channel_t channel;
    adc_oneshot_io_to_channel(POTI_PIN, &unit, &channel);

    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {.unit_id = unit};
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, channel, &config);

    while (1)
    {
        int64_t now = esp_timer_get_time();

        // -------- ZOOM --------
        bool zoom_btn = (gpio_get_level(ZOOM_BTN) == 0);

        if (zoom_btn && !last_zoom_btn)
        {
            zoom_index = (zoom_index + 1) % 3;
            zoom_level = zoom_steps[zoom_index];

            P6_set_zoom(zoom_level);
            ESP_LOGI(TAG, "Zoom %.1fx", zoom_level / 10.0);
        }
        last_zoom_btn = zoom_btn;

        // -------- PALETTE TOGGLE --------
        bool palette_btn = (gpio_get_level(PALETTE_BTN) == 0);

        if (palette_btn && !last_palette_btn)
        {
            palette_state = !palette_state;
            P6_set_palette(palette_state);
        }
        last_palette_btn = palette_btn;

        // -------- RECORD BUTTON (FLASH FEEDBACK) --------
        bool flash_btn = (gpio_get_level(FLASH_BTN) == 0);

        if (flash_btn && !last_flash_btn)
        {
            recording = !recording;

            if (recording)
            {
                // START recording → 3 slower flashes
                flash_feedback(3, 120);
                ESP_LOGI(TAG, "Recording START");
            }
            else
            {
                // STOP recording → 5 fast flashes
                flash_feedback(5, 50);
                ESP_LOGI(TAG, "Recording STOP");
            }
        }

        last_flash_btn = flash_btn;

        // -------- MULTI BUTTON --------
        bool multi_btn = (gpio_get_level(MULTI_BTN) == 0);

        if (multi_btn && !last_multi_btn)
        {
            press_time = now;
            nuc_triggered = false;
        }

        if (!multi_btn && last_multi_btn)
        {
            if (!nuc_triggered)
            {
                palette_index = (palette_index + 1) % 6;
                P6_set_palette_index(palette_index);
            }
        }

        // Long press → NUC
        if (multi_btn && !nuc_triggered && (now - press_time > 3000000))
        {
            P6_nuc();
            nuc_triggered = true;
        }

        last_multi_btn = multi_btn;

        // -------- ADC --------
        int adc_raw;
        adc_oneshot_read(adc_handle, channel, &adc_raw);

        adc_raw = (adc_raw * 2 + last_adc_val) / 3;

        bool moved = abs(adc_raw - last_adc_val) > 8;

        if (multi_btn)
        {
            if (moved)
            {
                contrast = 20 + ((adc_raw * 60) / 4095); // CLAMPED
                P6_set_contrast(contrast);
            }
        }
        else
        {
            if (moved)
            {
                brightness = 30 + ((adc_raw * 50) / 4095); // CLAMPED
                P6_set_brightness(brightness);
            }
        }

        last_adc_val = adc_raw;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ---------------- MAIN ----------------
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

    uart_init();

    vTaskDelay(pdMS_TO_TICKS(2500));

    // 🔥 Improved defaults
    P6_set_palette_index(0);
    P6_set_zoom(10);
    P6_set_brightness(64);
    P6_set_contrast(64);
    P6_set_denoise(64);
    P6_set_enhancement(10);
    P6_set_auto_nuc(30000);

    xTaskCreate(loop_task, "loop", 8192, NULL, 5, NULL);
}
