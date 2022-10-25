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

#define SUBSTATE_NULL 0
#define SUBSTATE_LOADING_CURVE_LIST 1
#define SUBSTATE_WAITING 2
#define SUBSTATE_LOADING_CURVE 3

#define CURVE_LIST_LENGTH 6
#define FILENAME_LENGTH 20

#define MAX_POINTS 10

static const char TAG[] = "conc_curves_state";

void ConcCurves::enter(Sctp* sctp)
{
	substate = SUBSTATE_LOADING_CURVE_LIST;
	sctp_lcd_clear();
	cursor = CURSOR_NULL;

	curve_list = (curve_t *) malloc (CURVE_LIST_LENGTH * sizeof(curve_t));
	assert(curve_list != NULL);
	sctp_flash_nvs_load_curve
}

void ConcCurves::okay(Sctp* sctp)
{
	switch (substate) {
		case SUBSTATE_WAITING: {
			if (cursor <= CURSOR_CURVE_5) {
				ESP_LOGI(TAG, "curve %d selected", cursor);
				// save selected curve in another variable
				sctp->curve = curve_list[cursor];
				sctp->curve.filename = (char *) malloc( 20 * sizeof(char) );
				strcpy( sctp->curve.filename, curve_list[cursor].filename );

				// load concentration and absorbance from selected curve
				sctp_lcd_conc_curves_loading_floats(cursor);

				sctp->curve.absorbance = (float *) malloc( MAX_POINTS * sizeof(float) );
				sctp->curve.concentration = (float *) malloc( MAX_POINTS * sizeof(float) );

				// queue check for loadConcFloats starts
				substate = SUBSTATE_LOADING_CURVE;

				// packing param
				((taskParam_t *) taskParam)->curve_ptr = &sctp->curve;
				((taskParam_t *) taskParam)->report_queue = report_queue;
				xTaskCreatePinnedToCore(loadConcFloats, "loadConcFloats", 2048, taskParam, 4, &taskHandle, 1);   
			}
			else if (cursor <= CURSOR_DEL_CURVE_5) {
				curve_list[cursor - CURVE_LIST_LENGTH].wavelength = 0;
				curve_list[cursor - CURVE_LIST_LENGTH].points = 0;

				// are these necessary?
				if ( curve_list[cursor - CURVE_LIST_LENGTH].absorbance != NULL) {
					free(curve_list[cursor - CURVE_LIST_LENGTH].absorbance);
					curve_list[cursor - CURVE_LIST_LENGTH].absorbance = NULL;
				}
				if ( curve_list[cursor - CURVE_LIST_LENGTH].concentration != NULL) {
					free(curve_list[cursor - CURVE_LIST_LENGTH].concentration);
					curve_list[cursor - CURVE_LIST_LENGTH].concentration = NULL;
				}
				
				// todo delete the memory from sd card
    			// xTaskCreatePinnedToCore(delConcCurveWrapper, "curve delete", 2048, sctp, 4, NULL, 1);   
				sctp_lcd_conc_curves_list(cursor, curve_list);
			}
			else if (cursor == CURSOR_BACK) {
				sctp->setState(Menu::getInstance());
			}
			break;
		}
	}
}

void ConcCurves::arrowDown(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING_CURVE_LIST: {
		// 	cursor = CURSOR_BACK;
		// 	sctp_lcd_conc_curves_opening(cursor);
		// 	break;
		// }
		case SUBSTATE_WAITING: {
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
			break;
		}
	}
}

void ConcCurves::arrowUp(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING_CURVE_LIST: {
		// 	cursor = CURSOR_BACK;
		// 	sctp_lcd_conc_curves_opening(cursor);
		// 	break;
		// }
		case SUBSTATE_WAITING: {
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
			break;
		}
	}
}

void ConcCurves::arrowRight(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING_CURVE_LIST: {
		// 	cursor = CURSOR_BACK;
		// 	sctp_lcd_conc_curves_opening(cursor);
		// 	break;
		// }
		case SUBSTATE_WAITING: {
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
			break;
		}
	}
}

void ConcCurves::arrowLeft(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING_CURVE_LIST: {
		// 	cursor = CURSOR_BACK;
		// 	sctp_lcd_conc_curves_opening(cursor);
		// 	break;
		// }
		case SUBSTATE_WAITING: {
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
			break;
		}
	}
}

void ConcCurves::exit(Sctp * sctp) {
	// free state buffers
	for (int i=0; i<CURVE_LIST_LENGTH; i++) {
		free(curve_list[i].filename);
	}
	free(curve_list);
	curve_list = NULL;
	free(taskParam);
	taskParam = NULL;
	vQueueDelete(report_queue);
	report_queue = NULL;
}

SctpState& ConcCurves::getInstance()
{
	static ConcCurves singleton;
	return singleton;
}