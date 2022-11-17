#include <esp_log.h>

#include "conc_save_state.h"
#include "conc_table_state.h"
#include "sctp_common_types.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_SAVING 1
#define SUBSTATE_WAITING 2

static const char * TAG = "conc_save_state";

void ConcSave::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_OK;

	// report_queue = xQueueCreate(1, sizeof(esp_err_t));
    substate = SUBSTATE_SAVING;

	sctp_lcd_spec_save_saving();
	sctp->curve.filename = (char*) malloc(25 * sizeof(char));
	sctp_flash_save_curve(sctp->curve);

	sctp_lcd_spec_save_finish(sctp->curve.filename);
	substate = SUBSTATE_WAITING;
}

void ConcSave::arrowDown(Sctp* sctp)
{
	if (substate == SUBSTATE_WAITING) {
		cursor = CURSOR_OK;
		sctp_lcd_spec_save_finish_cursor(cursor);
	}
}

void ConcSave::okay(Sctp* sctp)
{
	if (cursor == CURSOR_OK) {
        sctp->setState(ConcTable::getInstance());
	}
}

SctpState& ConcSave::getInstance()
{
	static ConcSave singleton;
	return singleton;
}