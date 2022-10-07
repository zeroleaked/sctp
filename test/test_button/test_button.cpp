#include <unity.h>
// pio test --filter test_button -vvv
// built-in libraries
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include "freertos/queue.h"

// external libraries
#include <button.h>
#include <pcf8574.h>

// internal libraries
#include "sctp_buttons.h"
#include "sctp_lcd.h"

static const char TAG[] = "main";
i2c_dev_t pcf;
uint8_t pcf_val = 0xFF;
uint8_t button;

void buttons_task() {
    for (;;) {
        // button_event_t ev;
        // if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {
        //     if ((ev.pin == BUTTON_PIN_DOWN) && ((ev.event == BUTTON_DOWN) || ev.event == BUTTON_HELD)) {
        //         ESP_LOGI(TAG, "button 1 pressed");
        //         sctp.arrowDown();

        //     }
        //     if ((ev.pin == BUTTON_PIN_OK) && ((ev.event == BUTTON_DOWN) || ev.event == BUTTON_HELD)) {
        //         ESP_LOGI(TAG, "button 2 pressed");
        //         sctp.okay();
        //     }
        //     ESP_LOGI(TAG, "state=%d", sctp.getCurrentStateId());
        // }
        pcf8574_port_read(&pcf, &pcf_val);
        if ((pcf_val&1)==0) {
            ESP_LOGI(TAG, "button LEFT pressed");
            // sctp.arrowLeft();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>1)&1)==0) {
            ESP_LOGI(TAG, "button RIGHT pressed");
            // sctp.arrowRight();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>2)&1)==0) {
            ESP_LOGI(TAG, "button UP pressed");
            // sctp.arrowUp();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>3)&1)==0) {
            ESP_LOGI(TAG, "button OK pressed");
            // sctp.okay();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>4)&1)==0) {
            ESP_LOGI(TAG, "button DOWN pressed");
            // sctp.arrowDown();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        // ESP_LOGI(TAG, "state=%d", sctp.getCurrentStateId());
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

void test1() {
    ESP_LOGI(TAG, "HELLO WORLD :)");

    i2cdev_init();
    pcf8574_init(&pcf, &pcf_val);
    
    buttons_task();
}

extern "C" {

void app_main(void);

}

void app_main() {
    UNITY_BEGIN();
    RUN_TEST(test1);
    UNITY_END();
}