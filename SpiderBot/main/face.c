#include "face.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "face";

#define SDA_GPIO 33
#define SCL_GPIO 35
#define OLED_W   128
#define OLED_H   64
#define I2C_PORT I2C_NUM_0

static ssd1306_handle_t oled = NULL;

static void draw_filled_circle(int cx, int cy, int r) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < OLED_W && py >= 0 && py < OLED_H) {
                    ssd1306_fill_point(oled, px, py, 1);
                }
            }
        }
    }
}

static void clear_rect(int x0, int y0, int w, int h) {
    for (int y = y0; y < y0 + h && y < OLED_H; y++) {
        for (int x = x0; x < x0 + w && x < OLED_W; x++) {
            if (x >= 0 && y >= 0) {
                ssd1306_fill_point(oled, x, y, 0);
            }
        }
    }
}

static void clear_screen(void) {
    ssd1306_clear_screen(oled, 0x00);
}

static void refresh(void) {
    ssd1306_refresh_gram(oled);
}

void face_init(void) {
    ESP_LOGI(TAG, "Init OLED SSD1306");

    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_GPIO,
        .scl_io_num = SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));

    oled = ssd1306_create(I2C_PORT, 0x3C);
    ssd1306_clear_screen(oled, 0x00);
    ssd1306_refresh_gram(oled);

    face_normal();
}

void face_normal(void) {
    clear_screen();
    // Yeux
    draw_filled_circle(35, 22, 10);
    draw_filled_circle(93, 22, 10);
    // Pupilles
    clear_rect(33, 20, 5, 5);
    clear_rect(91, 20, 5, 5);
    // Sourire
    for (int x = 35; x <= 93; x++) {
        int y = 48 + (x - 64) * (x - 64) / 150;
        if (y < OLED_H && y >= 0) {
            ssd1306_fill_point(oled, x, y, 1);
            ssd1306_fill_point(oled, x, y + 1, 1);
        }
    }
    refresh();
}