#include <string.h>
#include <esp_log.h>

#include "conc_curves_state.h"
#include "conc_table_state.h"
#include "../menu_state.h"
#include "conc_wavelength_state.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_CURVE_0 0
#define CURSOR_CURVE_1 1
#define CURSOR_CURVE_2 2
#define CURSOR_CURVE_3 3
#define CURSOR_CURVE_4 4
#define CURSOR_CURVE_5 5
#define CURSOR_DEL_CURVE_0 6
#define CURSOR_DEL_CURVE_1 7
#define CURSOR_DEL_CURVE_2 8
#define CURSOR_DEL_CURVE_3 9
#define CURSOR_DEL_CURVE_4 10
#define CURSOR_DEL_CURVE_5 11
#define CURSOR_BACK 12
#define CURSOR_NULL 13

#define CURVE_LIST_LENGTH 6

#define MAX_POINTS 10

static const char TAG[] = "conc_curves_state";

void ConcCurves::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = 0;

	curve_list = (curve_t *) malloc (CURVE_LIST_LENGTH * sizeof(curve_t));
	assert(curve_list != NULL);
	sctp_flash_nvs_load_curve_list(curve_list);
	for(int i=0; i < CURVE_LIST_LENGTH; i++) {
		if(curve_list[i].wavelength == 0) {
			curve_list[i].points = 0;
			for(int j=0; j < MAX_POINTS; j++) {
				curve_list[i].concentration = (float*) malloc(MAX_POINTS * sizeof(float));
				curve_list[i].absorbance = (float*) malloc(MAX_POINTS * sizeof(float));
				curve_list[i].concentration[j] = 0;
				curve_list[i].absorbance[j] = 0;
			}
		}
	}

	sctp_lcd_conc_curves_list(cursor, curve_list);
}

void ConcCurves::okay(Sctp* sctp)
{
	if (cursor <= CURSOR_CURVE_5) {
		ESP_LOGI(TAG, "curve %d selected", cursor);
		// save selected curve in another variable
		sctp->curve = curve_list[cursor];
		sctp->curve.absorbance = (float *) malloc( MAX_POINTS * sizeof(float) );
		sctp->curve.concentration = (float *) malloc( MAX_POINTS * sizeof(float) );

		sctp_flash_nvs_load_curve(&sctp->curve);

		if (sctp->curve.wavelength == 0) {
			sctp->setState(ConcWavelength::getInstance());
		}
		else {
			sctp->setState(ConcTable::getInstance());
		}

	}
	else if (cursor <= CURSOR_DEL_CURVE_5) {
		uint8_t curve_id = cursor - CURVE_LIST_LENGTH;

		curve_list[curve_id].wavelength = 0;
		curve_list[curve_id].points = 0;
		curve_list[curve_id].absorbance = (float *) malloc( MAX_POINTS * sizeof(float) );
		curve_list[curve_id].concentration = (float *) malloc( MAX_POINTS * sizeof(float) );
		for (int i=0; i < MAX_POINTS; i++) {
			curve_list[curve_id].absorbance[i] = -1;
			curve_list[curve_id].concentration[i] = 0;
		}

		sctp_flash_nvs_save_curve(&curve_list[curve_id]);
		free(curve_list[curve_id].absorbance);
		free(curve_list[curve_id].concentration);

		sctp_lcd_conc_curves_list_clear(cursor);
		cursor = cursor - 6;
		sctp_lcd_conc_curves_list_clear(cursor);
		sctp_lcd_conc_curves_list(cursor, curve_list);
	}
	else if (cursor == CURSOR_BACK) {
		sctp->setState(Menu::getInstance());
	}
}

void ConcCurves::arrowDown(Sctp* sctp)
{
	sctp_lcd_conc_curves_list_clear(cursor);
	if (cursor < CURSOR_CURVE_5) {
		cursor++;
	}
	else if (cursor == CURSOR_CURVE_5) {
		cursor = CURSOR_CURVE_0;
	}
	else if (cursor <= CURSOR_DEL_CURVE_5) {
		// do nothing	
	}
	else if (cursor == CURSOR_BACK) {
		cursor = CURSOR_CURVE_0;
	}
	else if (cursor == CURSOR_NULL) {
		cursor = CURSOR_CURVE_0;
	}
	sctp_lcd_conc_curves_list(cursor, curve_list);
}

void ConcCurves::arrowUp(Sctp* sctp)
{
	sctp_lcd_conc_curves_list_clear(cursor);
	if (cursor == CURSOR_CURVE_0) {
		cursor = CURSOR_CURVE_5;
	}
	else if (cursor <= CURSOR_CURVE_5) {
		cursor--;
	}
	else if (cursor <= CURSOR_DEL_CURVE_5) {
		// do nothing	
	}
	else if (cursor == CURSOR_BACK) {
		cursor = CURSOR_CURVE_0;
	}
	else if (cursor == CURSOR_NULL) {
		cursor = CURSOR_CURVE_0;
	}
	sctp_lcd_conc_curves_list(cursor, curve_list);
}

void ConcCurves::arrowRight(Sctp* sctp)
{
	sctp_lcd_conc_curves_list_clear(cursor);
	if (cursor <= CURSOR_CURVE_5) {
		if (curve_list[cursor].wavelength != 0) cursor += CURVE_LIST_LENGTH; // to del
	}
	else if (cursor < CURSOR_DEL_CURVE_5) {
		cursor = cursor - CURVE_LIST_LENGTH;
	}
	else if (cursor == CURSOR_DEL_CURVE_5) {
		cursor = CURSOR_BACK;
	}
	else if (cursor == CURSOR_BACK) {
		cursor = CURSOR_CURVE_5;
	}
	else if (cursor == CURSOR_NULL) {
		cursor = CURSOR_CURVE_0;
	}
	sctp_lcd_conc_curves_list(cursor, curve_list);
}

void ConcCurves::arrowLeft(Sctp* sctp)
{
	sctp_lcd_conc_curves_list_clear(cursor);
	if (cursor < CURSOR_CURVE_5) {
		if (curve_list[cursor].wavelength != 0) cursor += CURVE_LIST_LENGTH;
	}
	else if (cursor == CURSOR_CURVE_5) {
		cursor = CURSOR_BACK;
	}
	else if (cursor <= CURSOR_DEL_CURVE_5) {
		cursor = cursor - CURVE_LIST_LENGTH;
	}
	else if (cursor == CURSOR_BACK) {
		if (curve_list[CURSOR_CURVE_5].wavelength != 0) cursor = CURSOR_DEL_CURVE_5;
		else cursor = CURSOR_CURVE_5;
	}
	else if (cursor == CURSOR_NULL) {
		cursor = CURSOR_CURVE_0;
	}
	sctp_lcd_conc_curves_list(cursor, curve_list);
}

void ConcCurves::exit(Sctp * sctp) {
	// free state buffers
	free(curve_list);
	curve_list = NULL;
	sctp->lastPointIsInterpolated = false;
}

SctpState& ConcCurves::getInstance()
{
	static ConcCurves singleton;
	return singleton;
}