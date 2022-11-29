#include <unity.h>
#include <esp_log.h>

#include "../state_id.h"

#include "sctp.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv

static const char TAG[] = "sctp_states_test";

void history() {
    Sctp sctp2;
    sctp2.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp2.getCurrentStateId());
    sctp2.arrowDown();
    sctp2.arrowDown();
    sctp2.okay();
    TEST_ASSERT_EQUAL(STATE_HISTORY, sctp2.getCurrentStateId());
    sctp2.okay();
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();
    RUN_TEST(history);
    UNITY_END();
}