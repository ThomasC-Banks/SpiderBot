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
    int hip;
    int leg;
    int hip_base;
    int hip_range;      // positif = hanche va vers exterieur (base - range)
    int leg_base;
    int leg_range;      // sens OPPOSE a la hanche (base + range)
} leg_t;

static leg_t legs[NUM_LEGS] = {
    { "Avant Droite",   0, 1, 80, -30, 100, -30 },  // R1+R2 inversés
    { "Avant Gauche",   2, 3, 80, 30, 100, 30 },     // OK
    { "Arriere Droite", 5, 4, 85, 20, 95,  20 },     // a tester
    { "Arriere Gauche", 6, 7, 85, 20, 95, -20 },     // L4 inversé
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
    ESP_LOGI(TAG, "=== SETUP POSITION DE BASE ===");
    for (int i = 0; i < NUM_LEGS; i++) {
        ESP_LOGI(TAG, "  %s : hanche=%d, jambe=%d",
                legs[i].name, legs[i].hip_base, legs[i].leg_base);
        servo_move_smooth(legs[i].hip, legs[i].hip_base);
        servo_move_smooth(legs[i].leg, legs[i].leg_base);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
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

    // SETUP : position de base visible
    all_base();
    ESP_LOGI(TAG, "=== Base OK, attente 3s avant test ===");
    vTaskDelay(pdMS_TO_TICKS(3000));

    while (1) {
        for (int i = 0; i < NUM_LEGS; i++) {
            // Hanche va vers exterieur : base - range
            int hip_move = legs[i].hip_base - legs[i].hip_range;
            // Jambe va dans le sens OPPOSE : base + range
            int leg_move = legs[i].leg_base + legs[i].leg_range;

            ESP_LOGI(TAG, "=== %s ===", legs[i].name);
            ESP_LOGI(TAG, "  Hanche %d -> %d, Jambe %d -> %d",
                    legs[i].hip_base, hip_move,
                    legs[i].leg_base, leg_move);

            // 1. Lever la jambe (sens oppose a la hanche)
            ESP_LOGI(TAG, "  Lever jambe");
            servo_move_smooth(legs[i].leg, leg_move);
            vTaskDelay(pdMS_TO_TICKS(300));

            // 2. Hanche vers exterieur
            ESP_LOGI(TAG, "  Hanche exterieur");
            servo_move_smooth(legs[i].hip, hip_move);
            vTaskDelay(pdMS_TO_TICKS(300));

            // 3. Poser la jambe
            ESP_LOGI(TAG, "  Poser jambe");
            servo_move_smooth(legs[i].leg, legs[i].leg_base);
            vTaskDelay(pdMS_TO_TICKS(300));

            // 4. Retour hanche a la base
            ESP_LOGI(TAG, "  Retour base");
            servo_move_smooth(legs[i].hip, legs[i].hip_base);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        ESP_LOGI(TAG, "=== Cycle termine, retour base ===");
        all_base();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}