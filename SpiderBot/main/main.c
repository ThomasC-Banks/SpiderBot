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
    all_neutral();
    face_normal();
    wait(2000);

    while (1) {
        face_happy();
        dance_wave();
        wait(500);

        face_look_left();
        dance_twist();
        wait(500);

        face_excited();
        dance_bounce();
        wait(500);

        face_sleepy();
        dance_pushup();
        wait(500);

        face_wink();
        dance_wiggle();
        wait(500);

        face_look_right();
        dance_shimmy();
        wait(500);

        face_normal();
        dance_spin();
        wait(500);

        face_excited();
        dance_legs_up();
        wait(500);

        face_happy();
        dance_mexican_wave();
        wait(500);

        face_wink();
        dance_six_seven();
        wait(500);

        face_blink();
        wait(300);
        face_normal();
        wait(300);
        face_blink();
        wait(300);

        all_neutral();
        face_normal();
        ESP_LOGI(TAG, "=== REPLAY ===");
        wait(3000);
    }
}