#include <unity.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "sctp.h"

#include "concrete_sctp_states/spec_blank_state.h"

//platformio test --environment esp32doit-devkit-v1 -vvv

typedef enum {
    NONE,
    STATE_IDLE,
    STATE_MENU,
    STATE_SPEC_BLANK,
    STATE_CONC_CURVES,
    STATE_HISTORY,
    STATE_SETTINGS,
    STATE_SPEC_SAMPLE

} state_id_t;

static const char TAG[] = "sctp_states_test";


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

#define SPEC_BLANK_SUBSTATE_WAITING 0
#define SPEC_BLANK_SUBSTATE_SAMPLING 1
void spec_blank_test() {
    // test to Menu
    Sctp sctp0;
    // xTaskCreatePinnedToCore(sctp0.commandHandlerWrapper, "command handler", 2048, &sctp0, 3, &sctp0.task_command_handler, 1);
    sctp0.okay();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_BLANK, sctp0.getCurrentStateId());
    sctp0.arrowDown();
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_MENU, sctp0.getCurrentStateId());
    sctp0.okay();
    TEST_ASSERT_EQUAL(STATE_SPEC_BLANK, sctp0.getCurrentStateId());

    // test to SpecSample
    SpecBlank * specBlank = (SpecBlank *) sctp0.getCurrentState();
    TEST_ASSERT_EQUAL(SPEC_BLANK_SUBSTATE_WAITING, specBlank->substate);
    sctp0.arrowLeft();
    sctp0.arrowUp();
    sctp0.okay();
    TEST_ASSERT_EQUAL(SPEC_BLANK_SUBSTATE_SAMPLING, specBlank->substate);
    // TaskHandle_t task;
    vTaskDelay(6000 / portTICK_RATE_MS);
    TEST_ASSERT_EQUAL(STATE_SPEC_SAMPLE, sctp0.getCurrentStateId());
    // cleanup after testing
    free(sctp0.blank_sample.readout);

    // test cancel when sampling
    Sctp sctp1;
    // xTaskCreatePinnedToCore(sctp1.commandHandlerWrapper, "command handler", 2048, &sctp1, 3, &sctp1.task_command_handler, 1);
    sctp1.okay();    // to menu
    sctp1.okay();   // to SpecBlank
    specBlank = (SpecBlank *) sctp1.getCurrentState();  
    TEST_ASSERT_EQUAL(SPEC_BLANK_SUBSTATE_WAITING, specBlank->substate);
    sctp1.okay();
    TEST_ASSERT_EQUAL(SPEC_BLANK_SUBSTATE_SAMPLING, specBlank->substate);
    vTaskDelay(1000 / portTICK_RATE_MS);
    sctp1.arrowLeft();
    sctp1.okay();
    TEST_ASSERT_EQUAL(1, specBlank->cursor);
    TEST_ASSERT_EQUAL(SPEC_BLANK_SUBSTATE_WAITING, specBlank->substate);
    TEST_ASSERT_EQUAL(NULL, sctp1.blank_sample.readout);
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    ESP_LOGI(TAG, "hello world");

    RUN_TEST(idle_test);
    RUN_TEST(menu_test);
    RUN_TEST(spec_blank_test);

    UNITY_END();
}