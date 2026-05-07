#include <stdio.h>
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "spider-bot";

#define SERVO_MIN_PULSEWIDTH_US 500
#define SERVO_MAX_PULSEWIDTH_US 2500
#define SERVO_MAX_DEGREE        180
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT
#define LEDC_FREQUENCY          50
#define NUM_SERVOS              8

static const int servo_gpios[NUM_SERVOS] = {
    1, 2, 4, 6, 8, 10, 13, 14
};

static int servo_angles[NUM_SERVOS];

static const int base_angles[NUM_SERVOS] = {
    90, 40, 90, 0, 90, 90, 90, 180
//  R1  R2  L1 L2 R4  R3  L3  L4
};

static uint32_t angle_to_duty(uint32_t angle) {
    uint32_t pulse_us = SERVO_MIN_PULSEWIDTH_US + (angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US)) / SERVO_MAX_DEGREE;
    return (pulse_us * 16384) / 20000;
}

static void servo_set(int id, int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    ledc_set_duty(LEDC_MODE, (ledc_channel_t)id, angle_to_duty(angle));
    ledc_update_duty(LEDC_MODE, (ledc_channel_t)id);
    servo_angles[id] = angle;
}

static void servo_move_smooth(int id, int target) {
    if (target < 0) target = 0;
    if (target > 180) target = 180;
    int current = servo_angles[id];
    int step = (target > current) ? 1 : -1;
    while (current != target) {
        current += step;
        ledc_set_duty(LEDC_MODE, (ledc_channel_t)id, angle_to_duty(current));
        ledc_update_duty(LEDC_MODE, (ledc_channel_t)id);
        servo_angles[id] = current;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void wait(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void all_neutral(void) {
    ESP_LOGI(TAG, "POSITION DE BASE");
    for (int i = 0; i < NUM_SERVOS; i++) {
        servo_set(i, base_angles[i]);
    }
    wait(500);
}

static void dance_wave(void) {
    ESP_LOGI(TAG, "WAVE");
    servo_move_smooth(1, 10);
    wait(400);
    servo_move_smooth(1, 40);
    wait(300);
    servo_move_smooth(3, 30);
    wait(400);
    servo_move_smooth(3, 0);
    wait(300);
    servo_move_smooth(4, 60);
    wait(400);
    servo_move_smooth(4, 90);
    wait(300);
    servo_move_smooth(7, 150);
    wait(400);
    servo_move_smooth(7, 180);
    wait(300);
}

static void dance_twist(void) {
    ESP_LOGI(TAG, "TWIST");
    for (int rep = 0; rep < 4; rep++) {
        servo_move_smooth(0, 75);
        servo_move_smooth(2, 75);
        servo_move_smooth(5, 75);
        servo_move_smooth(6, 75);
        wait(250);
        servo_move_smooth(0, 105);
        servo_move_smooth(2, 105);
        servo_move_smooth(5, 105);
        servo_move_smooth(6, 105);
        wait(250);
    }
    servo_move_smooth(0, 90);
    servo_move_smooth(2, 90);
    servo_move_smooth(5, 90);
    servo_move_smooth(6, 90);
    wait(300);
}

static void dance_bounce(void) {
    ESP_LOGI(TAG, "BOUNCE");
    for (int rep = 0; rep < 5; rep++) {
        servo_move_smooth(1, 20);
        servo_move_smooth(3, 20);
        servo_move_smooth(4, 70);
        servo_move_smooth(7, 160);
        wait(200);
        servo_move_smooth(1, 40);
        servo_move_smooth(3, 0);
        servo_move_smooth(4, 90);
        servo_move_smooth(7, 180);
        wait(200);
    }
    wait(300);
}

static void dance_shimmy(void) {
    ESP_LOGI(TAG, "SHIMMY");
    for (int rep = 0; rep < 6; rep++) {
        servo_move_smooth(1, 25);
        servo_move_smooth(3, 15);
        servo_move_smooth(4, 75);
        servo_move_smooth(7, 165);
        wait(150);
        servo_move_smooth(1, 55);
        servo_move_smooth(3, 0);
        servo_move_smooth(4, 105);
        servo_move_smooth(7, 180);
        wait(150);
    }
    servo_move_smooth(1, 40);
    servo_move_smooth(3, 0);
    servo_move_smooth(4, 90);
    servo_move_smooth(7, 180);
    wait(300);
}

static void dance_spin(void) {
    ESP_LOGI(TAG, "SPIN");
    for (int rep = 0; rep < 3; rep++) {
        for (int offset = 0; offset <= 15; offset += 2) {
            servo_set(0, 90 + offset);
            servo_set(2, 90 + offset);
            servo_set(5, 90 - offset);
            servo_set(6, 90 - offset);
            wait(30);
        }
        for (int offset = 15; offset >= 0; offset -= 2) {
            servo_set(0, 90 + offset);
            servo_set(2, 90 + offset);
            servo_set(5, 90 - offset);
            servo_set(6, 90 - offset);
            wait(30);
        }
    }
    servo_move_smooth(0, 90);
    servo_move_smooth(2, 90);
    servo_move_smooth(5, 90);
    servo_move_smooth(6, 90);
    wait(300);
}

static void dance_legs_up(void) {
    ESP_LOGI(TAG, "LEGS UP");
    servo_move_smooth(1, 10);
    servo_move_smooth(3, 30);
    servo_move_smooth(4, 60);
    servo_move_smooth(7, 150);
    wait(500);
    servo_move_smooth(0, 75);
    servo_move_smooth(2, 75);
    servo_move_smooth(5, 105);
    servo_move_smooth(6, 105);
    wait(300);
    servo_move_smooth(0, 105);
    servo_move_smooth(2, 105);
    servo_move_smooth(5, 75);
    servo_move_smooth(6, 75);
    wait(300);
    servo_move_smooth(0, 90);
    servo_move_smooth(2, 90);
    servo_move_smooth(5, 90);
    servo_move_smooth(6, 90);
    servo_move_smooth(1, 40);
    servo_move_smooth(3, 0);
    servo_move_smooth(4, 90);
    servo_move_smooth(7, 180);
    wait(500);
}

void app_main(void) {
    ESP_LOGI(TAG, "Spider-Bot DANCE MODE");
    vTaskDelay(pdMS_TO_TICKS(3000));

    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    for (int i = 0; i < NUM_SERVOS; i++) {
        ledc_channel_config_t ch_cfg = {
            .speed_mode = LEDC_MODE,
            .channel = (ledc_channel_t)i,
            .timer_sel = LEDC_TIMER_0,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = servo_gpios[i],
            .duty = 0,
            .hpoint = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));
    }

    for (int i = 0; i < NUM_SERVOS; i++) {
        servo_angles[i] = base_angles[i];
    }
    all_neutral();
    wait(2000);

    while (1) {
        dance_wave();
        wait(1000);
        dance_twist();
        wait(1000);
        dance_bounce();
        wait(1000);
        dance_shimmy();
        wait(1000);
        dance_spin();
        wait(1000);
        dance_legs_up();
        wait(1000);
        all_neutral();
        ESP_LOGI(TAG, "=== REPLAY ===");
        wait(3000);
    }
}