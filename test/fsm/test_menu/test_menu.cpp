#include <unity.h>
#include <esp_log.h>

#include "../state_id.h"

#include "sctp.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv


static const char TAG[] = "sctp_states_test";


void spectrum() {
    Sctp sctp0;
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_BLANK, sctp0.getCurrentStateId());
}

void concentration() {
    Sctp sctp1;
    sctp1.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp1.getCurrentStateId());
    sctp1.arrowDown();
    sctp1.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp1.getCurrentStateId());
}

void history() {
    Sctp sctp2;
    sctp2.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp2.getCurrentStateId());
    sctp2.arrowDown();
    sctp2.arrowDown();
    sctp2.okay();
    TEST_ASSERT_EQUAL(STATE_HISTORY, sctp2.getCurrentStateId());
}

void settings() {
    Sctp sctp0;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp0.getCurrentStateId());
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
}



extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    RUN_TEST(spectrum);
    RUN_TEST(concentration);
    RUN_TEST(history);
    RUN_TEST(settings);

    UNITY_END();
}