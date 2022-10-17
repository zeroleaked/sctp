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

typedef struct {
	QueueHandle_t report_queue;
	curve_t * curve_ptr;
} taskParam_t;

static void loadConcCurveList(void * pvParameters) {
	// unpack param
	taskParam_t * task_param = (taskParam_t *) pvParameters;

	esp_err_t report = sctp_flash_load_curve_list(task_param->curve_ptr);

	assert(xQueueSend(task_param->report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "loadConcCurves() sended to queue");
	vTaskDelete( NULL );
}

static void loadConcFloats(void * pvParameters) {
	curve_t * curve = ((taskParam_t *) pvParameters)->curve_ptr;
	QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;

	esp_err_t report = sctp_flash_load_curve_floats(curve);
	
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "loadConcFloats() sended to queue");
	vTaskDelete( NULL );
}

void ConcCurves::enter(Sctp* sctp)
{
	substate = SUBSTATE_NULL;
	sctp_lcd_clear();
	cursor = CURSOR_NULL;

	sctp_lcd_conc_curves_opening(cursor);

	curve_list = (curve_t *) malloc (CURVE_LIST_LENGTH * sizeof(curve_t));
	assert(curve_list != NULL);
	for (int i=0; i<CURVE_LIST_LENGTH; i++) {
		curve_list[i].filename = (char *) malloc ( FILENAME_LENGTH * sizeof(char));
		assert(curve_list[i].filename != NULL);
	}

	report_queue = xQueueCreate(1, sizeof(esp_err_t));
	// queue check for loadConcCurveList starts
	substate = SUBSTATE_LOADING_CURVE_LIST;

	// packing param
	taskParam = (taskParam_t *) malloc (sizeof(taskParam_t));
	((taskParam_t *) taskParam)->curve_ptr = curve_list;
	((taskParam_t *) taskParam)->report_queue = report_queue;

    xTaskCreatePinnedToCore(loadConcCurveList, "loadConcCurveList", 2048, taskParam, 4, &taskHandle, 1);
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

void ConcCurves::refreshLcd(Sctp* sctp, command_t command) {
	// if (command == CURVES_LOAD) {
	// 	substate = SUBSTATE_WAITING;
	// 	sctp_lcd_conc_curves_list(cursor, curves);
	// }
	// else
	if (substate == SUBSTATE_LOADING_CURVE_LIST) {
		esp_err_t report;
		if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
			if (report == ESP_OK) {

				substate = SUBSTATE_WAITING;
				sctp_lcd_conc_curves_list(cursor, curve_list);

			}
		}
	}
	else if (substate == SUBSTATE_LOADING_CURVE) {
		esp_err_t report;
		if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
			substate = SUBSTATE_NULL;
			if (report == ESP_OK) {

				if (sctp->curve.wavelength == 0) {
					sctp->setState(ConcWavelength::getInstance());
				} 
				else {
					sctp->setState(ConcTable::getInstance());
				}

			}
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