#include <unity.h>

#include <esp_log.h>

#include "sctp.h"

//platformio test --environment esp32doit-devkit-v1 -vvv

typedef enum {
    NONE,
    STATE_IDLE,
    STATE_MENU,
    STATE_SPEC_BLANK,
    STATE_CONC_CURVES,
    STATE_HISTORY,
    STATE_SETTINGS

} state_id_t;

static const char TAG[] = "sctp_states";


void idle_test() {
    Sctp sctp0;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp0.getCurrentStateId());
    sctp0.arrowDown();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());

    Sctp sctp1;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp1.getCurrentStateId());
    sctp1.arrowLeft();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp1.getCurrentStateId());

    Sctp sctp2;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp2.getCurrentStateId());
    sctp2.arrowUp();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp2.getCurrentStateId());

    Sctp sctp3;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp3.getCurrentStateId());
    sctp3.arrowRight();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp3.getCurrentStateId());
    
    Sctp sctp4;
    TEST_ASSERT_EQUAL(STATE_IDLE, sctp4.getCurrentStateId());
    sctp4.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp4.getCurrentStateId());
}

void menu_test() {
    Sctp sctp0;
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_BLANK, sctp0.getCurrentStateId());

    Sctp sctp1;
    sctp1.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp1.getCurrentStateId());
    sctp1.arrowDown();
    sctp1.okay();
    TEST_ASSERT_EQUAL(STATE_CONC_CURVES, sctp1.getCurrentStateId());

    Sctp sctp2;
    sctp2.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp2.getCurrentStateId());
    sctp2.arrowDown();
    sctp2.arrowDown();
    sctp2.okay();
    TEST_ASSERT_EQUAL(STATE_HISTORY, sctp2.getCurrentStateId());

    Sctp sctp3;
    sctp3.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp3.getCurrentStateId());
    sctp3.arrowDown();
    sctp3.arrowDown();
    sctp3.arrowDown();
    sctp3.okay();
    TEST_ASSERT_EQUAL(STATE_SETTINGS, sctp3.getCurrentStateId());
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    RUN_TEST(idle_test);
    RUN_TEST(menu_test);

    UNITY_END();
}