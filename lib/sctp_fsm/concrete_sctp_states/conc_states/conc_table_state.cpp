#include <esp_log.h>

#include "conc_table_state.h"
#include "conc_blank_state.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define SUBSTATE_LOADING 0
#define SUBSTATE_CURSOR 1
#define SUBSTATE_CONCENTRATION 2

#define CURSOR_CONC_0 0
#define CURSOR_CONC_1 1
#define CURSOR_CONC_2 2
#define CURSOR_CONC_3 3
#define CURSOR_ABSORBANCE_0 4
#define CURSOR_ABSORBANCE_1 5
#define CURSOR_ABSORBANCE_2 6
#define CURSOR_ABSORBANCE_3 7
#define CURSOR_REGRESS 8
#define CURSOR_SAVE 9
#define CURSOR_BACK 10
#define CURSOR_NULL 11

#define MAX_POINTS 15

static const char TAG[] = "conc_table_state";

typedef struct {
	QueueHandle_t report_queue;
	curve_t * curve;
} taskParam_t;

static void loadConcFloats(void * pvParameters) {
	curve_t * curve = ((taskParam_t *) pvParameters)->curve;
	QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;

	esp_err_t report = sctp_flash_load_curve_floats(curve);
	
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "loadConcFloats() sended to queue");
	vTaskDelete( NULL );
}

void ConcTable::enter(Sctp * sctp) {
	sctp_lcd_clear();

	cursor = CURSOR_NULL;
	substate = SUBSTATE_LOADING;

	sctp_lcd_conc_table_opening(cursor);

	sctp->curve.absorbance = (float *) malloc( MAX_POINTS * sizeof(float) );
	sctp->curve.concentration = (float *) malloc( MAX_POINTS * sizeof(float) );

	report_queue = xQueueCreate(1, sizeof(esp_err_t));

	// packing param
	taskParam = (taskParam_t *) malloc (sizeof(taskParam_t));
	((taskParam_t *) taskParam)->curve = &sctp->curve;
	((taskParam_t *) taskParam)->report_queue = report_queue;
    xTaskCreatePinnedToCore(loadConcFloats, "loadConcFloats", 2048, taskParam, 4, &taskHandle, 1);   
}

void ConcTable::okay(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_LOADING: {
			break;
		}
		case SUBSTATE_CURSOR: {
			if (cursor == CURSOR_NULL) {}
			else if (cursor <= CURSOR_CONC_3) {
				substate = SUBSTATE_CONCENTRATION;
			}
			else if (cursor <= CURSOR_ABSORBANCE_3) {
				if (sctp->blank_take == NULL) {
                    sctp->setState(ConcBlank::getInstance());
				}
				// todo check blank, then sample
			}
			else if (cursor == CURSOR_REGRESS) {
				// todo check sufficient length
			}
			else if (cursor == CURSOR_SAVE) {
				// todo ConcState
			}
			else if (cursor == CURSOR_BACK) {
				// todo free all buffers as if going to menu
			}
			break;
		}
		case SUBSTATE_CONCENTRATION: {
			substate = SUBSTATE_CURSOR;
			if ( (sctp->curve.concentration != 0) && (sctp->curve.absorbance != 0) ) {
				sctp->curve.points++;
			}
			else if ((sctp->curve.concentration == 0) && (sctp->curve.absorbance == 0)) {
				sctp->curve.points--;
			}
			break;
		}
	}
}

void ConcTable::arrowDown(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_LOADING: {
			break;
		}
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);

			if (cursor == CURSOR_NULL) {
				cursor = CURSOR_CONC_0;
			}
			else if (cursor < CURSOR_CONC_3) {
				if ( cursor == sctp->curve.points ) { // cursor at add new
					cursor = CURSOR_SAVE;
				}
				else {
					cursor++;
				}
			}
			else if (cursor == CURSOR_CONC_3) {
				if (cursor + row_offset == sctp->curve.points) { // cursor at add new
					cursor = CURSOR_SAVE;
				}
				else if (row_offset == MAX_POINTS - 4) { // table full
					cursor = CURSOR_SAVE;
				}
				else {
					row_offset++;
				}
			}
			else if (cursor < CURSOR_ABSORBANCE_3) {
				if ( cursor - 4 == sctp->curve.points ) { // cursor at add new
					cursor = CURSOR_REGRESS;
				}
				else {
					cursor++;
				}
			}
			else if (cursor == CURSOR_ABSORBANCE_3) {
				if (cursor + row_offset == sctp->curve.points) { // cursor at add new
					cursor = CURSOR_REGRESS;
				}
				else if (row_offset == MAX_POINTS - 4) { // table full
					cursor = CURSOR_REGRESS;
				}
				else {
					row_offset++;
				}
			}
			else if (cursor == CURSOR_REGRESS) {
				cursor = CURSOR_ABSORBANCE_0;
				row_offset = 0;
			}
			else if (cursor == CURSOR_SAVE) {
				cursor = CURSOR_CONC_0;
				row_offset = 0;
			}
			else if (cursor == CURSOR_BACK) {
				cursor = CURSOR_CONC_0;
				row_offset = 0;
			}
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
			break;
		}
		case SUBSTATE_CONCENTRATION: {
			sctp->curve.concentration[row_offset + cursor] = sctp->curve.concentration[row_offset + cursor] - 0.001;
			sctp_lcd_conc_table_concentration(cursor, sctp->curve.concentration[row_offset + cursor]);
			break;
		}
	}
}

void ConcTable::arrowUp(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_LOADING: {
			break;
		}
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);

			if (cursor == CURSOR_NULL) {
				cursor = CURSOR_CONC_0;
			}
			else if (cursor == CURSOR_CONC_0) {
				if ( row_offset == 0 ) {
					cursor = CURSOR_SAVE;
				}
				else {
					row_offset--;
				}
			}
			else if (cursor <= CURSOR_CONC_3) {
				cursor--;
			}
			else if (cursor == CURSOR_ABSORBANCE_0) {
				if ( row_offset == 0 ) {
					cursor = CURSOR_REGRESS;
				}
				else {
					row_offset--;
				}
			}
			else if (cursor <= CURSOR_ABSORBANCE_3) {
				cursor--;
			}
			else if (cursor == CURSOR_REGRESS) {
				if (sctp->curve.points >= 4) {
					cursor = CURSOR_ABSORBANCE_3;
				}
				else {
					cursor = sctp->curve.points + 4;
				}
			}
			else if (cursor == CURSOR_SAVE) {
				if (sctp->curve.points >= 4) {
					cursor = CURSOR_CONC_3;
				}
				else {
					cursor = sctp->curve.points;
				}
			}
			else if (cursor == CURSOR_BACK) {
				if (sctp->curve.points >= 4) {
					cursor = CURSOR_CONC_3;
				}
				else {
					cursor = sctp->curve.points;
				}
			}
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
			break;
		}
		case SUBSTATE_CONCENTRATION: {
			sctp->curve.concentration[row_offset + cursor] += 0.001;
			sctp_lcd_conc_table_concentration(cursor, sctp->curve.concentration[row_offset + cursor]);
			break;
		}
	}
}

void ConcTable::arrowRight(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_LOADING: {
			break;
		}
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);

			if (cursor == CURSOR_NULL) {
				cursor = CURSOR_CONC_0;
			}
			else if (cursor <= CURSOR_CONC_3) {
				cursor += 4;
			}
			else if (cursor <= CURSOR_ABSORBANCE_3) {
				cursor = cursor - 4;
			}
			else if (cursor == CURSOR_REGRESS) {
				cursor = CURSOR_BACK;
			}
			else if (cursor == CURSOR_SAVE) {
				cursor = CURSOR_REGRESS;
			}
			else if (cursor == CURSOR_BACK) {
				cursor = CURSOR_SAVE;
			}
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
			break;
		}
	}
}

void ConcTable::arrowLeft(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_LOADING: {
			break;
		}
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);

			if (cursor == CURSOR_NULL) {
				cursor = CURSOR_CONC_0;
			}
			else if (cursor <= CURSOR_CONC_3) {
				cursor += 4;
			}
			else if (cursor <= CURSOR_ABSORBANCE_3) {
				cursor = cursor - 4;
			}
			else if (cursor == CURSOR_REGRESS) {
				cursor = CURSOR_SAVE;
			}
			else if (cursor == CURSOR_SAVE) {
				cursor = CURSOR_BACK;
			}
			else if (cursor == CURSOR_BACK) {
				cursor = CURSOR_REGRESS;
			}
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
			break;
		}
	}
}


void ConcTable::refreshLcd(Sctp* sctp, command_t command) {
	if (substate == SUBSTATE_LOADING) {
		esp_err_t report;
		if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
			if (report == ESP_OK) {
				free(taskParam);
				taskParam = NULL;
				vQueueDelete(report_queue);
				report_queue = NULL;

				substate = SUBSTATE_CURSOR;
				cursor = CURSOR_CONC_0;
				sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
			}
		}
	}
};

SctpState& ConcTable::getInstance()
{
	static ConcTable singleton;
	return singleton;
}