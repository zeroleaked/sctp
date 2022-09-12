#include <unity.h>

#include <esp_log.h>

#include "sctp.h"

static const char TAG[] = "sctp_states";

Sctp sctp;

void idle_test() {
    TEST_ASSERT_EQUAL(0, sctp.getCurrentStateId());
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    RUN_TEST(idle_test);

    UNITY_END();
}