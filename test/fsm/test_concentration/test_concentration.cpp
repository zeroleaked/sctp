#include <unity.h>
#include <esp_log.h>

#include "../state_id.h"

#include "sctp.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv


static const char TAG[] = "sctp_states_test";

void concentration_mvp() {
    Sctp sctp0;
    sctp0.okay();
    ESP_LOGI(TAG, "entered Menu");
    sctp0.arrowDown();
    vTaskDelay(2000 / portTICK_RATE_MS);
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp0.getCurrentStateId());
    ESP_LOGI(TAG, "entered ConcCurves");
    vTaskDelay(3000 / portTICK_RATE_MS);
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.okay();
    vTaskDelay(3000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_CONC_WAVELENGTH, sctp0.getCurrentStateId());
    sctp0.arrowUp();
    sctp0.arrowUp();
    sctp0.okay();
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp0.getCurrentStateId());
    sctp0.arrowRight();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_BLANK, sctp0.getCurrentStateId());
    sctp0.okay();
    vTaskDelay(6000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_CONC_SAMPLE, sctp0.getCurrentStateId());
    sctp0.okay();
    vTaskDelay(7000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp0.getCurrentStateId());
}

void regress_test() {
    Sctp sctp0;
    sctp0.okay();
    ESP_LOGI(TAG, "entered Menu");
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp0.getCurrentStateId());
    ESP_LOGI(TAG, "entered ConcCurves");
    vTaskDelay(3000 / portTICK_RATE_MS);
    sctp0.arrowDown();
    sctp0.okay();
    vTaskDelay(3000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp0.getCurrentStateId());
    sctp0.arrowDown(); // to second point
    sctp0.arrowDown(); // to third point
    sctp0.arrowDown(); // to fourth point
    sctp0.arrowDown(); // to fifth point
    sctp0.arrowDown(); // to 6th point
    sctp0.arrowDown(); // to 7th point
    sctp0.arrowDown(); // to 8th point
    sctp0.arrowDown(); // to 9th point
    sctp0.arrowDown(); // to 10th point
    sctp0.arrowDown(); // to save
    sctp0.arrowRight(); // to regress
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_REGRESS, sctp0.getCurrentStateId());
    sctp0.arrowDown(); // to save
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp0.getCurrentStateId());
}

void curves_state () {

    Sctp s;
    Sctp * sctp = &s;

    TEST_ASSERT_EQUAL(STATE_IDLE, sctp->getCurrentStateId());
    sctp->okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp->getCurrentStateId());
    sctp->arrowDown();
    sctp->okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp->getCurrentStateId());

    vTaskDelay(5000/ portTICK_PERIOD_MS);
}

void table_state () {
    Sctp s;
    Sctp * sctp = &s;

    TEST_ASSERT_EQUAL(STATE_IDLE, sctp->getCurrentStateId());
    sctp->okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp->getCurrentStateId());
    sctp->arrowDown();
    sctp->okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp->getCurrentStateId());
    sctp->arrowDown();
    sctp->okay();
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp->getCurrentStateId());
    // TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp->getCurrentStateId());
}

void table_edit()
{
    Sctp sctp0;
    sctp0.okay();
    ESP_LOGI(TAG, "entered Menu");
    sctp0.arrowDown();
    vTaskDelay(2000 / portTICK_RATE_MS);
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp0.getCurrentStateId());
    ESP_LOGI(TAG, "entered ConcCurves");
    vTaskDelay(3000 / portTICK_RATE_MS);
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.arrowDown();
    sctp0.okay();
    vTaskDelay(3000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_CONC_WAVELENGTH, sctp0.getCurrentStateId());
    sctp0.arrowUp();
    sctp0.arrowUp();
    sctp0.okay();
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp0.getCurrentStateId());
    sctp0.okay();
    for(int i=0; i<5; i++) {
        sctp0.arrowUp();
    }
    sctp0.okay();
    sctp0.arrowDown();
    sctp0.okay();
    for (int i = 0; i < 10; i++)
    {
        sctp0.arrowUp();
    }
    sctp0.okay();
    sctp0.arrowDown();
    sctp0.okay();
    for (int i = 0; i < 15; i++)
    {
        sctp0.arrowUp();
    }
    sctp0.okay();
    sctp0.arrowDown();
    sctp0.okay();
    for (int i = 0; i < 15; i++)
    {
        sctp0.arrowUp();
    }
    sctp0.okay();
    sctp0.arrowDown();
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    // RUN_TEST(curves_state);
    // RUN_TEST(table_state);
    RUN_TEST(table_edit);
    // RUN_TEST(concentration_mvp);
    // RUN_TEST(regress_test);

    UNITY_END();
}