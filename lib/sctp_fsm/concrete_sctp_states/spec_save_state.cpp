#include <esp_log.h>

#include "spec_save_state.h"
#include "spec_result_state.h"
#include "sctp_lcd.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_SAVING 0
#define SUBSTATE_WAITING 1

static const char * TAG = "spec_save_state";

void SpecSave::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_SAVING;
	cursor = CURSOR_NULL;
	ESP_LOGI(TAG, "enter, saving...");
    xTaskCreatePinnedToCore(sctp->saveSpectrumWrapper, "spectrum save", 2048, sctp, 4, NULL, 1);
	sctp_lcd_spec_save_saving();
}

void SpecSave::arrowDown(Sctp* sctp)
{
	if (substate == SUBSTATE_WAITING) {
		cursor = CURSOR_OK;
		sctp_lcd_spec_save_finish_cursor(cursor);
	}
}

void SpecSave::okay(Sctp* sctp)
{
	if (cursor == CURSOR_OK) {
		sctp_lcd_spec_save_finish_cursor(cursor);
        sctp->setState(SpecResult::getInstance());
	}
}

void SpecSave::refreshLcd(Sctp* sctp)
{
	substate = SUBSTATE_WAITING;

    command_t command;
    assert(xQueueReceive(sctp->lcd_refresh_queue, &command, 0) == pdTRUE); // already peeked
    if (command == SPECTRUM_SAVE) {
		sctp_lcd_spec_save_finish(sctp->saved_name);
    }
}

SctpState& SpecSave::getInstance()
{
	static SpecSave singleton;
	return singleton;
}