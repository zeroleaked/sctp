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

void test1()
{
    uint8_t cursor = 0;
    sctp_lcd_clear();
    sctp_lcd_start();
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