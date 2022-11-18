#include <esp_log.h>

#include "history_list_state.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"
#include "menu_state.h"
#include "spec_states/spec_result_state.h"
#include "conc_states/conc_regress_state.h"

#define CURSOR_FILE_0 0
#define CURSOR_FILE_1 1
#define CURSOR_FILE_2 2
#define CURSOR_FILE_3 3
#define CURSOR_FILE_4 4
#define CURSOR_FILE_5 5
#define CURSOR_BACK 6
#define CURSOR_NULL 7

#define SUBSTATE_LIST 0
#define SUBSTATE_SPEC 1
#define SUBSTATE_CONC 2

#define MAX_FILES 60
#define MAX_POINTS 10

static const char TAG[] = "history_list_state";

void sctp_flash_history(Sctp * sctp) {
	sctp->history_list = (history_t *) malloc(MAX_FILES * sizeof(history_t));
	for(int i=0; i<MAX_FILES; i++) {
		sctp->history_list[i].filename = (char*) malloc(20 * sizeof(char));
	}
	if (sctp->history_list != NULL) {
		ESP_LOGI(TAG, "history list allocated");
	}
	else {
		ESP_LOGE(TAG, "failed to allocate history list");
		sctp->history_list_length = 0;
		return;
	}
	sctp_flash_load_history_list(sctp->history_list);

};

void sctp_history_regress(curve_t * curve, conc_regression_t * regress_line) {
	uint8_t standards_length;
	if (curve->concentration[curve->points - 1] == 0)
		standards_length = curve->points - 1;
	else
		standards_length = curve->points;
	ESP_LOGI(TAG, "detected %d standard sample points", standards_length);
	assert(standards_length <= MAX_POINTS); // MAX_POINTS is 10
	assert(curve->points <= MAX_POINTS);	// MAX_POINTS is 10
	// if (standards_length < 10)
	// { // last row is not a standard point
	// 	// check if we can interpolate
	// 	interpolate = (curve->absorbance[standards_length] != 0);
	// }
	// else
	// {
	// 	interpolate = false; // last row is also a standard. Not interpolating
	// }

	float sum_absorbance = 0;
	float sum_concentration = 0;
	float sum_absorbance_sq = 0;
	float sum_product = 0;
	for (int i = 0; i < standards_length; i++)
	{
		ESP_LOGI(TAG, "i=%d, (%f, %f)", i, curve->absorbance[i], curve->concentration[i]);
		sum_absorbance += curve->absorbance[i];
		sum_concentration += curve->concentration[i];
		sum_absorbance_sq += curve->absorbance[i] * curve->absorbance[i];
		sum_product += curve->absorbance[i] * curve->concentration[i];
	}
	float divider = standards_length * sum_absorbance_sq - sum_absorbance * sum_absorbance;
	assert(divider != 0);
	regress_line->gradient = (standards_length * sum_product - sum_concentration * sum_absorbance) / divider;
	regress_line->offset = (sum_concentration * sum_absorbance_sq - sum_absorbance * sum_product) / divider;
	ESP_LOGI(TAG, "m=%f, b=%f", regress_line->gradient, regress_line->offset);
};

void HistoryList::enter(Sctp* sctp)
{
	substate = SUBSTATE_LIST;
	sctp_lcd_clear();
	cursor = 0;
	offset = 0;

	filenames = (char (*) [25]) malloc(60*25*sizeof(char));
	file_count = 0;

	sctp_flash_history(sctp);
	for (int i = 0; i < MAX_FILES; i++)
	{
		if (sctp->history_list[i].filename[0] != '\0') {
			strcpy(filenames[i], sctp->history_list[i].filename);
			file_count++;
		}
	}

	sctp_lcd_history_list(cursor, offset, filenames);
}

void HistoryList::okay(Sctp* sctp)
{
	switch(substate) {
		case SUBSTATE_LIST: {
			if ( cursor < CURSOR_BACK ) {
				sctp->history_index = cursor + offset;

				// copy selected history
				sctp->history = sctp->history_list[sctp->history_index];

				if ( sctp->history.measurement_mode == MEASUREMENT_MODE_CONCENTRATION ) {
					// history_curve = (curve_t*) malloc (sizeof(curve_t));
					sctp->curve.filename = (char*) malloc (25 * sizeof(char));
					sctp->curve.concentration = (float *)malloc(10 * sizeof(float));
					sctp->curve.absorbance = (float *)malloc(10 * sizeof(float));
					strcpy(sctp->curve.filename, sctp->history.filename);
					sctp_flash_load_curve_floats(&sctp->curve);

					regress_line = (conc_regression_t *)malloc(sizeof(conc_regression_t));
					sctp_history_regress(&sctp->curve, regress_line);
					ESP_LOGI(TAG, "m=%f, b=%f", regress_line->gradient, regress_line->offset);

					sctp_lcd_clear();
					sctp_lcd_conc_regress(cursor, sctp->curve, interpolate, regress_line);
					substate = SUBSTATE_CONC;
					free(regress_line);
					free(sctp->curve.filename);
					free(sctp->curve.absorbance);
					ESP_LOGI(TAG, "free 1 done");
					sctp->curve.absorbance = NULL;
					free(sctp->curve.concentration);
					ESP_LOGI(TAG, "free 2 done");
					sctp->curve.concentration = NULL;
				}
				else {
					history_wavelength = (float*) malloc(450 * sizeof(float));
					history_absorbance = (float*) malloc(450 * sizeof(float));
					spectrum_length = (uint16_t*) malloc(sizeof(uint16_t));
					sctp_flash_load_spectrum(sctp->history.filename, history_absorbance, history_wavelength, spectrum_length);
					ESP_LOGI(TAG, "spec file loading passed.");

					sctp_lcd_clear();
					sctp_lcd_spec_result_full(history_wavelength, history_absorbance, *spectrum_length);
					ESP_LOGI(TAG, "lcd passed.");
					substate = SUBSTATE_SPEC;
					ESP_LOGI(TAG, "changing substate passed.");
					free(spectrum_length);
					free(history_absorbance);
					free(history_wavelength);
				}
			}
			else switch (cursor) {
				case CURSOR_BACK: {
					sctp->setState(Menu::getInstance());
					break;
				}
				case CURSOR_NULL: { break; } // do nothing
			}
			break;
		}
		case SUBSTATE_SPEC: {
			substate = SUBSTATE_LIST;
			sctp_lcd_clear();
			offset = 0;
			sctp_lcd_history_list(cursor, offset, filenames);
			break;
		}
		case SUBSTATE_CONC:
		{
			substate = SUBSTATE_LIST;
			sctp_lcd_clear();
			offset = 0;
			sctp_lcd_history_list(cursor, offset, filenames);
			break;
		}
	}
}

void HistoryList::arrowUp(Sctp* sctp)
{
	if(substate == SUBSTATE_LIST) {
		sctp_lcd_history_list_clear(cursor);
		if (cursor == CURSOR_FILE_0)
		{
			if (offset == 0)
			{
				cursor = CURSOR_BACK;
			}
			else
			{
				offset--;
			}
		}
		else if (cursor <= CURSOR_FILE_5)
		{
			cursor--;
		}
		else if (cursor == CURSOR_BACK)
		{
			if (file_count >= 6)
			{
				cursor = CURSOR_FILE_5;
			}
			else
			{
				cursor = file_count;
			}
		}
		sctp_lcd_history_list(cursor, offset, filenames);
	}
	else if (substate == SUBSTATE_SPEC)
	{
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
	else if(substate == SUBSTATE_CONC) {
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
}

void HistoryList::arrowDown(Sctp* sctp)
{
	if(substate == SUBSTATE_LIST) {
		sctp_lcd_history_list_clear(cursor);

		if (cursor == CURSOR_FILE_5)
		{
			if (cursor + offset == file_count)
			{
				cursor = CURSOR_BACK;
			}
			else if (offset == MAX_FILES - 6)
			{
				cursor = CURSOR_BACK;
			}
			else {
				offset++;
			}
		}
		else if (cursor < CURSOR_FILE_5)
		{
			if (cursor + offset == file_count)
			{
				cursor = CURSOR_BACK;
			}
			else
			{
				offset++;
			}
		}
		else if (substate == SUBSTATE_SPEC)
		{
			substate = SUBSTATE_LIST;
			sctp_lcd_clear();
			offset = 0;
			sctp_lcd_history_list(cursor, offset, filenames);
		}
		else if (cursor == CURSOR_BACK)
		{
			cursor = CURSOR_FILE_0;
			offset = 0;
		}
		sctp_lcd_history_list(cursor, offset, filenames);
	}
	else if (substate == SUBSTATE_CONC)
	{
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
}

void HistoryList::arrowRight(Sctp *sctp)
{
	if (substate == SUBSTATE_CONC)
	{
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
	else if (substate == SUBSTATE_SPEC)
	{
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
}

void HistoryList::arrowLeft(Sctp *sctp)
{
	if (substate == SUBSTATE_CONC)
	{
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
	else if (substate == SUBSTATE_SPEC)
	{
		substate = SUBSTATE_LIST;
		sctp_lcd_clear();
		offset = 0;
		sctp_lcd_history_list(cursor, offset, filenames);
	}
}

void HistoryList::exit(Sctp* sctp)
{
	free(filenames);
	for(int i=0; i<MAX_FILES; i++) {
		free(sctp->history_list[i].filename);
	}
	free(sctp->history_list);
	sctp->history_list = NULL;
	sctp->history_list_length = 0;
}

SctpState& HistoryList::getInstance()
{
	static HistoryList singleton;
	return singleton;
}