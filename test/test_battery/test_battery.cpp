#include <unity.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <i2cdev.h>

#include "sctp_battery.h"

//pio test --filter test_battery -vvv

#define PIN_LAMP_SWITCH GPIO_NUM_16

static const char TAG[] = "test_battery";

const int SCCB_I2C_PORT         = 0;



void test_read() {
    i2cdev_init();

    sctp_battery_init();

    uint8_t battery_percentage;

    gpio_config_t conf = {};
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = 1LL << PIN_LAMP_SWITCH;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);
    gpio_set_level( PIN_LAMP_SWITCH, 0);

    for (int i=0; i<5; i++) {
        sctp_battery_sample(&battery_percentage);
        ESP_LOGI(TAG, "bat = %d", battery_percentage);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    gpio_set_level( PIN_LAMP_SWITCH, 1);

    for (int i=0; i<60; i++) {
        sctp_battery_sample(&battery_percentage);
        ESP_LOGI(TAG, "bat = %d", battery_percentage);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    gpio_set_level( PIN_LAMP_SWITCH, 0);

    for (int i=0; i<5; i++) {
        sctp_battery_sample(&battery_percentage);
        ESP_LOGI(TAG, "bat = %d", battery_percentage);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

extern "C" {

void app_main();
}

void app_main() {
    UNITY_BEGIN();

    RUN_TEST(test_read);

    UNITY_END();
}