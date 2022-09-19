#include <esp_log.h>

#include "spec_result_state.h"
#include "spec_save_state.h"
#include "spec_sample_state.h"
#include "spec_blank_state.h"
#include "../menu_state.h"
#include "sctp_lcd.h"

#define CURSOR_SAVE 0
#define CURSOR_RESAMPLE 1
#define CURSOR_REBLANK 2
#define CURSOR_MENU 3

static const char TAG[] = "spec_result_state";

// // command dummies
// void sctp_save_spec();

void SpecResult::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_SAVE;
	if (sctp->spectrum_wavelength == NULL) {
		sctp->spectrum_wavelength = (float *) malloc(sizeof(float) * sctp->calibration.length);
		for (int i=0; i<sctp->calibration.length; i++) {
			sctp->spectrum_wavelength[i] = (float)(i + sctp->calibration.start) * sctp->calibration.gain + sctp->calibration.bias;
		}
	}
	sctp_lcd_spec_result(cursor, sctp->spectrum_wavelength, sctp->absorbance, sctp->calibration.length);
}

void SpecResult::okay(Sctp* sctp)
{
	switch (cursor) {
        case CURSOR_SAVE: {
			sctp->setState(SpecSave::getInstance());
            break;
        } 
		case CURSOR_RESAMPLE: {
			free(sctp->sample_take);
			sctp->sample_take = NULL;
			free(sctp->absorbance);
			sctp->absorbance = NULL;
			sctp->setState(SpecSample::getInstance());
			break;
		}
		case CURSOR_REBLANK: {
			free(sctp->sample_take);
			sctp->sample_take = NULL;
			free(sctp->absorbance);
			sctp->absorbance = NULL;
			free(sctp->blank_take->readout);
			sctp->blank_take->readout = NULL;
			free(sctp->blank_take);
			sctp->blank_take = NULL;
			sctp->setState(SpecBlank::getInstance());
			break;
		}
		case CURSOR_MENU: {
			free(sctp->sample_take);
			sctp->sample_take = NULL;
			free(sctp->absorbance);
			sctp->absorbance = NULL;
			free(sctp->blank_take->readout);
			sctp->blank_take->readout = NULL;
			free(sctp->blank_take);
			sctp->blank_take = NULL;
			sctp->setState(Menu::getInstance());
			break;
		}
	}
}

void SpecResult::arrowUp(Sctp* sctp)
{
	sctp_lcd_spec_result_clear(this->cursor);
	if (cursor == CURSOR_SAVE) cursor = CURSOR_MENU;
    else cursor--;
	sctp_lcd_spec_result_cursor(cursor);
}

void SpecResult::arrowDown(Sctp* sctp)
{
	sctp_lcd_spec_result_clear(cursor);
	if (cursor == CURSOR_MENU) cursor = CURSOR_SAVE;
	else cursor++;
	sctp_lcd_spec_result(cursor, sctp->spectrum_wavelength, sctp->absorbance, sctp->calibration.length);
	// sctp_lcd_spec_result_cursor(cursor);
}

SctpState& SpecResult::getInstance()
{
	static SpecResult singleton;
	return singleton;
}