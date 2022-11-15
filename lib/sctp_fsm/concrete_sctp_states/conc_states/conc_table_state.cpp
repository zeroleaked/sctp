#include <esp_log.h>

#include "conc_table_state.h"
#include "conc_blank_state.h"
#include "conc_sample_state.h"
#include "conc_regress_state.h"
#include "conc_save_state.h"
#include "conc_curves_state.h"

#include "sctp_lcd.h"
#include "sctp_flash.h"

#define SUBSTATE_CURSOR 0
#define SUBSTATE_CONCENTRATION 1

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

#define MAX_POINTS 10

static const char TAG[] = "conc_table_state";

void ConcTable::enter(Sctp * sctp) {
	ESP_LOGI(TAG, "entered ConcTable");
	sctp_lcd_clear();
	ESP_LOGI(TAG, "pt1");

	cursor = CURSOR_CONC_0;
	ESP_LOGI(TAG, "pt2");
	substate = SUBSTATE_CURSOR;
	ESP_LOGI(TAG, "pt3");
	row_offset = 0;

	ESP_LOGI(TAG, "interpolated: %d", sctp->lastPointIsInterpolated);
	sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve, sctp->lastPointIsInterpolated);
	ESP_LOGI(TAG, "after entering LCD display");
}

void ConcTable::okay(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_CURSOR: {
			if (cursor <= CURSOR_CONC_3) {
				substate = SUBSTATE_CONCENTRATION;
			}
			else if (cursor <= CURSOR_ABSORBANCE_3) {
				sctp->point_sel = cursor - 4 + row_offset;
				// if(sctp->curve.points < sctp->point_sel)
				// 	sctp->curve.points++;
				if (sctp->blank_take == NULL) {
                    sctp->setState(ConcBlank::getInstance());
				}
				else {
                    sctp->setState(ConcSample::getInstance());
				}
			}
			else if (cursor == CURSOR_REGRESS) {
                sctp->setState(ConcRegress::getInstance());
			}
			else if (cursor == CURSOR_SAVE) {
                sctp->setState(ConcSave::getInstance());
			}
			else if (cursor == CURSOR_BACK) {
				// free blank buffers
				if (sctp->blank_take != NULL) {
					// if (sctp->blank_take->readout != NULL) free(sctp->blank_take->readout);
					// ESP_LOGI(TAG, "free 1 done");
					// free(sctp->blank_take);
					// ESP_LOGI(TAG, "free 2 done");
					sctp->blank_take = NULL;
				}

                free(sctp->curve.absorbance);
				ESP_LOGI(TAG, "free 3 done");
				sctp->curve.absorbance = NULL;
                free(sctp->curve.concentration);
				ESP_LOGI(TAG, "free 4 done");
				sctp->curve.concentration = NULL;

                sctp->setState(ConcCurves::getInstance());
			}
			break;
		}
		case SUBSTATE_CONCENTRATION: {
			substate = SUBSTATE_CURSOR;
			ESP_LOGI(TAG, "point %d changed to %f", row_offset + cursor, sctp->curve.concentration[row_offset+cursor]);
			if ( (sctp->curve.concentration[row_offset + cursor] >= 0.001) ) {
				ESP_LOGI(TAG, "%f != 0", sctp->curve.concentration[row_offset + cursor]);
				if (row_offset+cursor == sctp->curve.points)
					sctp->curve.points++;
				sctp->curve.absorbance[row_offset + cursor] = -1;
			}
			else if ((sctp->curve.concentration[row_offset + cursor] < 0.001) && (sctp->curve.absorbance[row_offset + cursor] < 0)) {
				ESP_LOGI(TAG, "points decr");
				if (row_offset+cursor != sctp->curve.points)
					sctp->curve.points--;
				sctp->curve.absorbance[row_offset + cursor] = -1;
				sctp->curve.concentration[row_offset + cursor] = 0;
			}
			sctp_lcd_clear();
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve, sctp->lastPointIsInterpolated);
			sctp_flash_nvs_save_curve(&sctp->curve);
			for (int i=0; i< sctp->curve.points; i++) {
				ESP_LOGI(TAG, "%d, %f, %f", i, sctp->curve.concentration[i], sctp->curve.absorbance[i]);
			}
			break;
		}
	}
}

void ConcTable::arrowDown(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_CURSOR: {
			
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);
			if (cursor < CURSOR_CONC_3) {
				if ( cursor + row_offset == sctp->curve.points ) { // cursor at add new
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
				if ( cursor - 4 + row_offset == sctp->curve.points ) { // cursor at add new
					cursor = CURSOR_REGRESS;
				}
				else {
					cursor++;
				}
			}
			else if (cursor == CURSOR_ABSORBANCE_3) {
				if (cursor - 4 + row_offset == sctp->curve.points) { // cursor at add new
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
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve, sctp->lastPointIsInterpolated);

			ESP_LOGI(TAG, "point %d of %d points", cursor+row_offset, sctp->curve.points);

			break;
		}
		case SUBSTATE_CONCENTRATION: {
			if (sctp->curve.concentration[row_offset + cursor] >= 0.001) {
				sctp->curve.concentration[row_offset + cursor] = sctp->curve.concentration[row_offset + cursor] - 0.001;
			} else {
				sctp->curve.concentration[row_offset + cursor] = 0;
			}
			sctp_lcd_conc_table_concentration(cursor, sctp->curve.concentration[row_offset + cursor]);
			break;
		}
	}
}

void ConcTable::arrowUp(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);
			if (cursor == CURSOR_CONC_0) {
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
					row_offset = sctp->curve.points - 3;
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
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve, sctp->lastPointIsInterpolated);

			ESP_LOGI(TAG, "point %d of %d points", cursor+row_offset, sctp->curve.points);

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
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);
			if (cursor <= CURSOR_CONC_3) {
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
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve, sctp->lastPointIsInterpolated);
			break;
		}
	}
}

void ConcTable::arrowLeft(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_CURSOR: {
			sctp_lcd_conc_table_clear(cursor, row_offset, sctp->curve);
			if (cursor <= CURSOR_CONC_3) {
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
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve, sctp->lastPointIsInterpolated);
			break;
		}
	}
}


void ConcTable::refreshLcd(Sctp* sctp, command_t command) {};

SctpState& ConcTable::getInstance()
{
	static ConcTable singleton;
	return singleton;
}