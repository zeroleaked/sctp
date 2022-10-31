#include <unity.h>
#include "sctp_lcd.h"
#include "math.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

static const char TAG[] = "test_lcd";

#define TFT_TOSCA 0x5D35
#define TFT_MUSTARD 0xD461

static LGFX display;
static LGFX_Sprite sprite(&display);

void test_dimension()
{
    ESP_LOGI(TAG, "HELLO WORLD :)");

    sctp_lcd_start();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    sctp_lcd_clear();
}

void test1()
{
    ESP_LOGI(TAG, "HELLO WORLD :)");

    sctp_lcd_start();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    sctp_lcd_clear();
    sctp_lcd_spec_blank_waiting(0, 100);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    sctp_lcd_clear();
    sctp_lcd_conc_sample_waiting(0, 100);
}

extern "C"
{

    void app_main(void);
}

void app_main()
{
    UNITY_BEGIN();
    RUN_TEST(test1);
    UNITY_END();
}