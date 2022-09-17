#include <unity.h>
#include <esp_log.h>

#include "../state_id.h"

#include "sctp.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv


static const char TAG[] = "sctp_states_test";


#define CONC_CURVES_SUBSTATE_LOADING 0
#define CONC_CURVES_SUBSTATE_WAITING 1
void concentration_mvp() {
    Sctp sctp0;
    sctp0.okay();
    ESP_LOGI(TAG, "entered Menu");
    sctp0.arrowDown();
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
    TEST_ASSERT_EQUAL(STATE_CONC_WAVELENGTH, sctp0.getCurrentStateId());
    sctp0.okay();
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_TABLE, sctp0.getCurrentStateId());
}



extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    RUN_TEST(concentration_mvp);

    UNITY_END();
}