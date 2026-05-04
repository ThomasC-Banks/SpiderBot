#include <stdio.h>
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "spider-bot";

#define SERVO_MIN_PULSEWIDTH_US 500
#define SERVO_MAX_PULSEWIDTH_US 2500
#define SERVO_MAX_DEGREE        180

#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES       LEDC_TIMER_14_BIT
#define LEDC_FREQUENCY      50

#define SERVO_GPIO          4  // GPIO 4

static uint32_t angle_to_duty(uint32_t angle) {
    uint32_t pulse_us = SERVO_MIN_PULSEWIDTH_US +
        (angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US))
        / SERVO_MAX_DEGREE;
    return (pulse_us * 16384) / 20000;
}

void app_main(void) {
    ESP_LOGI(TAG, "Spider-Bot - demarrage");

    // Attendre 3 secondes pour laisser l'USB CDC s'initialiser
    vTaskDelay(pdMS_TO_TICKS(3000));

    ESP_LOGI(TAG, "Init servo sur GPIO %d", SERVO_GPIO);

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&timer_cfg);
    ESP_LOGI(TAG, "Timer config: %s", esp_err_to_name(err));

    ledc_channel_config_t channel_cfg = {
        .speed_mode = LEDC_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = SERVO_GPIO,
        .duty       = 0,
        .hpoint     = 0,
    };
    err = ledc_channel_config(&channel_cfg);
    ESP_LOGI(TAG, "Channel config: %s", esp_err_to_name(err));

    ESP_LOGI(TAG, "Servo en mouvement...");

    while (1) {
        for (int angle = 0; angle <= 180; angle += 5) {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, angle_to_duty(angle));
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        for (int angle = 180; angle >= 0; angle -= 5) {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, angle_to_duty(angle));
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}