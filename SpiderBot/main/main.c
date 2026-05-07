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
#define NUM_LEGS                4

static const int servo_gpios[NUM_SERVOS] = {
    1, 2, 4, 6, 8, 10, 13, 14
};

typedef struct {
    const char *name;
    int hip;            // index servo hanche
    int leg;            // index servo jambe
    int hip_base;       // angle de base hanche
    int hip_range;      // amplitude hanche (positif = vers exterieur)
    int leg_base;       // angle de base jambe
    int leg_range;      // amplitude jambe (positif = lever)
} leg_t;

// Ajuste hip_base, hip_range, leg_base, leg_range par patte
static leg_t legs[NUM_LEGS] = {
    { "Avant Droite",   0, 1, 90, 20, 90, 25 },   // R1 + R2
    { "Avant Gauche",   2, 3, 90, 20, 90, 25 },   // L1 + L2
    { "Arriere Droite", 5, 4, 90, 15, 90, 15 },    // R3 + R4
    { "Arriere Gauche", 6, 7, 90, 15, 90, 15 },    // L3 + L4
};

static int servo_angles[NUM_SERVOS];

static uint32_t angle_to_duty(uint32_t angle) {
    uint32_t pulse_us = SERVO_MIN_PULSEWIDTH_US +
        (angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US))
        / SERVO_MAX_DEGREE;
    return (pulse_us * 16384) / 20000;
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

static void all_base(void) {
    ESP_LOGI(TAG, "Position de base");
    for (int i = 0; i < NUM_LEGS; i++) {
        servo_move_smooth(legs[i].hip, legs[i].hip_base);
        servo_move_smooth(legs[i].leg, legs[i].leg_base);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
}

void app_main(void) {
    ESP_LOGI(TAG, "Spider-Bot - Test patte par patte");
    vTaskDelay(pdMS_TO_TICKS(3000));

    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    for (int i = 0; i < NUM_SERVOS; i++) {
        ledc_channel_config_t ch_cfg = {
            .speed_mode = LEDC_MODE,
            .channel    = (ledc_channel_t)i,
            .timer_sel  = LEDC_TIMER_0,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = servo_gpios[i],
            .duty       = 0,
            .hpoint     = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));
        servo_angles[i] = 90;
    }

    all_base();
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        for (int i = 0; i < NUM_LEGS; i++) {
            int hip_out = legs[i].hip_base - legs[i].hip_range;
            int leg_up  = legs[i].leg_base - legs[i].leg_range;

            ESP_LOGI(TAG, "=== %s (hanche %d->%d, jambe %d->%d) ===",
                     legs[i].name,
                     legs[i].hip_base, hip_out,
                     legs[i].leg_base, leg_up);

            // 1. Lever la jambe
            servo_move_smooth(legs[i].leg, leg_up);
            vTaskDelay(pdMS_TO_TICKS(300));

            // 2. Hanche vers exterieur
            servo_move_smooth(legs[i].hip, hip_out);
            vTaskDelay(pdMS_TO_TICKS(300));

            // 3. Poser la jambe
            servo_move_smooth(legs[i].leg, legs[i].leg_base);
            vTaskDelay(pdMS_TO_TICKS(300));

            // 4. Retour base
            servo_move_smooth(legs[i].hip, legs[i].hip_base);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        ESP_LOGI(TAG, "=== Cycle termine ===");
        all_base();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}