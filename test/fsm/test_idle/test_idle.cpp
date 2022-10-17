#include <unity.h>
#include <esp_log.h>

#include "../state_id.h"

#include "sctp.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv


static const char TAG[] = "sctp_states_test";


void arrowDown() {
    Sctp sctp0;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp0.getCurrentStateId());
    sctp0.arrowDown();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
}

void arrowUp() {
    Sctp sctp0;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp0.getCurrentStateId());
    sctp0.arrowUp();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
}

void arrowLeft() {
    Sctp sctp0;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp0.getCurrentStateId());
    sctp0.arrowLeft();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
}

void arrowRight() {
    Sctp sctp0;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp0.getCurrentStateId());
    sctp0.arrowRight();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
}

void okay() {
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

    // RUN_TEST(arrowUp);
    // RUN_TEST(arrowDown);
    // RUN_TEST(arrowLeft);
    RUN_TEST(okay);
    RUN_TEST(arrowRight);

    UNITY_END();
}