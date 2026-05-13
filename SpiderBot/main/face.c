#include "face.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "ssd1306.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "face";

#define SDA_GPIO 33
#define SCL_GPIO 35
#define OLED_W   128
#define OLED_H   64

static ssd1306_handle_t oled = NULL;
static uint8_t framebuf[OLED_W * (OLED_H / 8)];

static void draw_filled_circle(uint8_t *buf, int cx, int cy, int r) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < OLED_W && py >= 0 && py < OLED_H) {
                    buf[(py / 8) * OLED_W + px] |= (1 << (py % 8));
                }
            }
        }
    }
}

static void draw_filled_rect(uint8_t *buf, int x0, int y0, int w, int h) {
    for (int y = y0; y < y0 + h && y < OLED_H; y++) {
        for (int x = x0; x < x0 + w && x < OLED_W; x++) {
            if (x >= 0 && y >= 0) {
                buf[(y / 8) * OLED_W + x] |= (1 << (y % 8));
            }
        }
    }
}

static void clear_rect(uint8_t *buf, int x0, int y0, int w, int h) {
    for (int y = y0; y < y0 + h && y < OLED_H; y++) {
        for (int x = x0; x < x0 + w && x < OLED_W; x++) {
            if (x >= 0 && y >= 0) {
                buf[(y / 8) * OLED_W + x] &= ~(1 << (y % 8));
            }
        }
    }
}

static void clear_buf(void) {
    memset(framebuf, 0, sizeof(framebuf));
}

static void flush_buf(void) {
    for (int page = 0; page < OLED_H / 8; page++) {
        ssd1306_write_page(oled, page, 0, OLED_W, &framebuf[page * OLED_W]);
    }
}

void face_init(void) {
    ESP_LOGI(TAG, "Init OLED SSD1306");

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_GPIO,
        .scl_io_num = SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    ssd1306_config_t oled_cfg = {
        .width = OLED_W,
        .height = OLED_H,
        .i2c_address = 0x3C,
        .i2c_bus = bus,
    };
    oled = ssd1306_create(&oled_cfg);
    ssd1306_clear_screen(oled);
    ssd1306_refresh_gram(oled);

    face_normal();
}

void face_normal(void) {
    clear_buf();
    draw_filled_circle(framebuf, 35, 32, 12);
    draw_filled_circle(framebuf, 93, 32, 12);
    flush_buf();
}

void face_happy(void) {
    clear_buf();
    draw_filled_circle(framebuf, 35, 28, 12);
    draw_filled_circle(framebuf, 93, 28, 12);
    for (int x = 30; x <= 98; x++) {
        int y = 50 + (x - 64) * (x - 64) / 200;
        if (y < OLED_H) {
            framebuf[(y / 8) * OLED_W + x] |= (1 << (y % 8));
        }
    }
    flush_buf();
}

void face_blink(void) {
    clear_buf();
    draw_filled_rect(framebuf, 23, 30, 24, 4);
    draw_filled_rect(framebuf, 81, 30, 24, 4);
    flush_buf();
}

void face_look_left(void) {
    clear_buf();
    draw_filled_circle(framebuf, 30, 32, 12);
    draw_filled_circle(framebuf, 88, 32, 12);
    flush_buf();
}

void face_look_right(void) {
    clear_buf();
    draw_filled_circle(framebuf, 40, 32, 12);
    draw_filled_circle(framebuf, 98, 32, 12);
    flush_buf();
}

void face_sleepy(void) {
    clear_buf();
    draw_filled_circle(framebuf, 35, 35, 12);
    draw_filled_circle(framebuf, 93, 35, 12);
    clear_rect(framebuf, 20, 0, 35, 32);
    clear_rect(framebuf, 78, 0, 35, 32);
    flush_buf();
}

void face_excited(void) {
    clear_buf();
    draw_filled_circle(framebuf, 35, 32, 16);
    draw_filled_circle(framebuf, 93, 32, 16);
    flush_buf();
}

void face_wink(void) {
    clear_buf();
    draw_filled_circle(framebuf, 35, 32, 12);
    draw_filled_rect(framebuf, 81, 30, 24, 4);
    flush_buf();
}