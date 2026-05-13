#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "servo.h"
#include "dance.h"
#include "face.h"

static const char *TAG = "spider-bot";

static void wait(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void app_main(void) {
    ESP_LOGI(TAG, "Spider-Bot DANCE MODE");
    vTaskDelay(pdMS_TO_TICKS(3000));

    servo_init();
    face_init();
    face_normal();
    all_neutral();
    wait(2000);

    while (1) {
        dance_wave();
        wait(1000);
        dance_twist();
        wait(1000);
        dance_bounce();
        wait(1000);
        dance_pushup();
        wait(1000);
        dance_wiggle();
        wait(1000);
        dance_shimmy();
        wait(1000);
        dance_spin();
        wait(1000);
        dance_legs_up();
        wait(1000);
        dance_mexican_wave();
        wait(1000);
        dance_six_seven();
        wait(1000);
        all_neutral();
        ESP_LOGI(TAG, "=== REPLAY ===");
        wait(3000);
    }
}