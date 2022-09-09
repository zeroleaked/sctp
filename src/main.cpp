// built-in libraries
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include "freertos/queue.h"

// external libraries
#include <button.h>

// internal libraries
#include "sctp.h"
#include "sctp_buttons.h"
#include "sctp_lcd.h"

static const char TAG[] = "main";

Sctp sctp;
QueueHandle_t button_events;

static void buttons_task(void *pvParameter) {
    for (;;) {
        button_event_t ev;
        if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {
            if ((ev.pin == BUTTON_PIN_DOWN) && ((ev.event == BUTTON_DOWN) || ev.event == BUTTON_HELD)) {
                ESP_LOGI(TAG, "button 1 pressed");
                sctp.arrowDown();

            }
            if ((ev.pin == BUTTON_PIN_OK) && ((ev.event == BUTTON_DOWN) || ev.event == BUTTON_HELD)) {
                ESP_LOGI(TAG, "button 2 pressed");
                sctp.okay();
            }
            ESP_LOGI(TAG, "state=%d", sctp.getCurrentStateId());
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

extern "C" {

void app_main(void);

}


void lcdTask(void*) {
    for(;;) {
    }
}


void app_main() {
    ESP_LOGI(TAG, "HELLO WORLD :)");
    
    sctp_buttons_init(&button_events);

    xTaskCreate(&buttons_task, "button_handler", CONFIG_ESP32_BUTTON_TASK_STACK_SIZE, NULL, 10, NULL);
    // xTaskCreatePinnedToCore( lcdTask, "lcdTask", 8192, NULL, 1, NULL, 1 );

}