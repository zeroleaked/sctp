#include <unity.h>
#include <esp_log.h>

#include "../state_id.h"

#include "sctp.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv


static const char TAG[] = "sctp_states_test";

void spectrum_mvp() {
    // test to SpecSample
    Sctp sctp0;
    sctp0.okay();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_BLANK, sctp0.getCurrentStateId());
    sctp0.arrowLeft();
    sctp0.arrowUp();
    sctp0.okay();
    ESP_LOGI(TAG, "substate sampling");
    // TaskHandle_t task;
    vTaskDelay(6000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "idle fin");
    TEST_ASSERT_EQUAL(STATE_SPEC_SAMPLE, sctp0.getCurrentStateId());

    // test to SpecResult
    sctp0.arrowLeft();
    sctp0.arrowUp();
    sctp0.okay();
    vTaskDelay(7000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_SPEC_RESULT, sctp0.getCurrentStateId());

    // test to SpecSave
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_SAVE, sctp0.getCurrentStateId());

    // back to SpecResult
    vTaskDelay(3000 / portTICK_RATE_MS);
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_RESULT, sctp0.getCurrentStateId());
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
    TEST_ASSERT_EQUAL(NULL, sctp0.sample_take);
    TEST_ASSERT_EQUAL(NULL, sctp0.absorbance);
    TEST_ASSERT_EQUAL(NULL, sctp0.blank_take);
}

void power_test()
{
    Sctp sctp0;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 300000 / portTICK_RATE_MS;
    xLastWakeTime = xTaskGetTickCount();
    sctp0.okay();
    for(;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        // test to SpecSample
        sctp0.okay();
        TEST_ASSERT_EQUAL(STATE_SPEC_BLANK, sctp0.getCurrentStateId());
        sctp0.arrowLeft();
        sctp0.arrowUp();
        sctp0.okay();
        ESP_LOGI(TAG, "substate sampling");
        // TaskHandle_t task;
        vTaskDelay(6000 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "idle fin");
        TEST_ASSERT_EQUAL(STATE_SPEC_SAMPLE, sctp0.getCurrentStateId());

        // test to SpecResult
        sctp0.arrowLeft();
        sctp0.arrowUp();
        sctp0.okay();
        vTaskDelay(7000 / portTICK_RATE_MS);
        TEST_ASSERT_EQUAL(STATE_SPEC_RESULT, sctp0.getCurrentStateId());

        // test to SpecSave
        sctp0.arrowDown();
        sctp0.okay();
        TEST_ASSERT_EQUAL(STATE_SPEC_SAVE, sctp0.getCurrentStateId());

        // back to SpecResult
        vTaskDelay(3000 / portTICK_RATE_MS);
        sctp0.arrowDown();
        sctp0.okay();
        TEST_ASSERT_EQUAL(STATE_SPEC_RESULT, sctp0.getCurrentStateId());
        sctp0.arrowDown();
        sctp0.arrowDown();
        sctp0.arrowDown();
        sctp0.arrowDown();
        sctp0.okay();
        TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
        TEST_ASSERT_EQUAL(NULL, sctp0.sample_take);
        TEST_ASSERT_EQUAL(NULL, sctp0.absorbance);
        TEST_ASSERT_EQUAL(NULL, sctp0.blank_take);
    }
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    RUN_TEST(spectrum_mvp);

    UNITY_END();
}