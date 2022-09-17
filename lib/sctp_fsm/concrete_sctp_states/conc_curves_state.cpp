#include <string.h>
#include <esp_log.h>
// #include <freertos/FreeRTOS.h>

#include "conc_curves_state.h"
#include "conc_table_state.h"
#include "menu_state.h"
// #include "conc_wavelength_state.h"
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

#define SUBSTATE_LOADING 0
#define SUBSTATE_WAITING 1

#define CURVE_LIST_LENGTH 6
#define FILENAME_LENGTH 20

static const char TAG[] = "conc_curves_state";

typedef struct {
	QueueHandle_t report_queue;
	curve_t * curve_list;
} taskParam_t;

static void loadConcCurveList(void * pvParameters) {
	// unpack param
	taskParam_t * task_param = (taskParam_t *) pvParameters;

	sctp_flash_load_curve_list(task_param->curve_list);

	command_t command = CURVES_LOAD;
	assert(xQueueSend(task_param->report_queue, &command, 0) == pdTRUE);
	ESP_LOGI(TAG, "loadConcCurves() sended to queue");

	vTaskDelete( NULL );
}

void ConcCurves::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;
	substate = SUBSTATE_LOADING;

	sctp_lcd_conc_curves_opening(cursor);

	curve_list = (curve_t *) malloc (CURVE_LIST_LENGTH * sizeof(curve_t));
	for (int i=0; i<CURVE_LIST_LENGTH; i++) {
		curve_list[i].filename = (char *) malloc ( FILENAME_LENGTH * sizeof(char));
	}

	report_queue = xQueueCreate(1, sizeof(uint8_t));

	ESP_LOGI(TAG, "packing param");
	// packing param
	taskParam = (taskParam_t *) malloc (sizeof(taskParam_t));
	((taskParam_t *) taskParam)->curve_list = curve_list;
	((taskParam_t *) taskParam)->report_queue = report_queue;

    xTaskCreatePinnedToCore(loadConcCurveList, "loadConcCurveList", 2048, taskParam, 4, &taskHandle, 1);

	ESP_LOGI(TAG, "enter() fin");   
}

void ConcCurves::okay(Sctp* sctp)
{
	switch (substate) {
		case SUBSTATE_LOADING: {
			// if (cursor == CURSOR_BACK) {
			// 	vTaskDelete(taskHandle);

			// 	// free state buffers
			// 	free(taskParam);
			// 	for (int i=0; i< CURVE_LIST_LENGTH; i++) {
			// 		free(curve_list[i].filename);
			// 	}
			// 	free(curve_list);
			// 	curve_list = NULL;
			// 	sctp->setState(Menu::getInstance());
			// }
		}
		case SUBSTATE_WAITING: {
			if (cursor <= CURSOR_CURVE_5) {
				ESP_LOGI(TAG, "curve %d selected", cursor);
				// save selected curve in another variable, free unselected curves' buffer
				sctp->curve = curve_list[cursor];
				sctp->curve.filename = (char *) malloc( 20 * sizeof(char) );
				strcpy( sctp->curve.filename, curve_list[cursor].filename );

				// free state buffers
				for (int i=0; i<CURVE_LIST_LENGTH; i++) {
					free(curve_list[i].filename);
				}
				free(curve_list);
				curve_list = NULL;
				sctp->setState(ConcTable::getInstance());
			}
			else if (cursor <= CURSOR_DEL_CURVE_5) {
				curve_list[cursor - CURVE_LIST_LENGTH].wavelength = 0;
				curve_list[cursor - CURVE_LIST_LENGTH].points = 0;

				if ( curve_list[cursor - 6].absorbance != NULL) {
					free(curve_list[cursor - 6].absorbance);
					curve_list[cursor - 6].absorbance = NULL;
				}
				
				if ( curve_list[cursor - 6].concentration != NULL) {
					free(curve_list[cursor - 6].concentration);
					curve_list[cursor - 6].concentration = NULL;
				}
				
    			// xTaskCreatePinnedToCore(sctp->delConcCurveWrapper, "curve delete", 2048, sctp, 4, NULL, 1);   
				sctp_lcd_conc_curves_list(cursor, curve_list);
			}
			else if (cursor == CURSOR_BACK) {
				for (int i=0; i< CURVE_LIST_LENGTH; i++) {
					free(curve_list[i].filename);
				}
				free(curve_list);
				curve_list = NULL;
				sctp->setState(Menu::getInstance());
			}
		}
	}
}

void ConcCurves::arrowDown(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING: {
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
				cursor = CURSOR_BACK;
			}
			else if (cursor < CURSOR_BACK) {
				cursor ++;
			}
			else if (cursor == CURSOR_BACK) {
				cursor = CURSOR_CURVE_0;
			}
			sctp_lcd_conc_curves_list_cursor(cursor);
			break;
		}
	}
}

void ConcCurves::arrowUp(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING: {
		// 	cursor = CURSOR_BACK;
		// 	sctp_lcd_conc_curves_opening(cursor);
		// 	break;
		// }
		case SUBSTATE_WAITING: {
			sctp_lcd_conc_curves_list_clear(cursor);
			if (cursor == CURSOR_CURVE_0) {
				cursor = CURSOR_BACK;
			}
			else if (cursor <= CURSOR_CURVE_5) {
				cursor--;
			}
			else if (cursor == CURSOR_DEL_CURVE_0) {
				cursor = CURSOR_BACK;
			}
			else if (cursor <= CURSOR_DEL_CURVE_5) {
				cursor--;
			}
			else if (cursor == CURSOR_BACK) {
				cursor = CURSOR_CURVE_5;
			}
			sctp_lcd_conc_curves_list_cursor(cursor);
			break;
		}
	}
}

void ConcCurves::arrowRight(Sctp* sctp)
{
	switch (substate) {
		// case SUBSTATE_LOADING: {
		// 	cursor = CURSOR_BACK;
		// 	sctp_lcd_conc_curves_opening(cursor);
		// 	break;
		// }
		case SUBSTATE_WAITING: {
			sctp_lcd_conc_curves_list_clear(cursor);
			if (cursor <= CURSOR_CURVE_5) {
				cursor += 6;
			}
			else if (cursor <= CURSOR_DEL_CURVE_5) {
				cursor = cursor - 6;
			}
			sctp_lcd_conc_curves_list_cursor(cursor);
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
	command_t intern_command;
	if (xQueueReceive(report_queue, &intern_command, 0) == pdTRUE) {
		ESP_LOGI(TAG, "intern_command received");
		free(taskParam);

		substate = SUBSTATE_WAITING;
		cursor = CURSOR_CURVE_0;
		sctp_lcd_conc_curves_list(cursor, curve_list);
	}
};

SctpState& ConcCurves::getInstance()
{
	static ConcCurves singleton;
	return singleton;
}