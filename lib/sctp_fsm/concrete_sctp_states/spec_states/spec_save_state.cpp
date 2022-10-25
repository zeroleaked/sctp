#include <esp_log.h>

#include "spec_save_state.h"
#include "spec_result_state.h"
#include "sctp_common_types.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_NULL 0
#define SUBSTATE_SAVING 1
#define SUBSTATE_WAITING 2

void SpecSave::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;

    substate = SUBSTATE_SAVING;
	
	sctp_lcd_spec_save_saving();

	sctp_flash_save_spectrum(sctp->absorbance, sctp->spectrum_wavelength, sctp->calibration.length);

	sctp_lcd_spec_save_finish(sctp->saved_name);
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
        sctp->setState(SpecResult::getInstance());
	}
}

SctpState& SpecSave::getInstance()
{
	static SpecSave singleton;
	return singleton;
}