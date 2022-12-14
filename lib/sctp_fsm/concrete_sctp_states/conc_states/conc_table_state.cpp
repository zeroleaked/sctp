#include <esp_log.h>

#include "conc_table_state.h"
#include "conc_blank_state.h"
#include "conc_sample_state.h"
#include "conc_regress_state.h"
#include "conc_save_state.h"
#include "conc_curves_state.h"
#include "sctp_lcd.h"

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
	sctp_lcd_clear();

	cursor = CURSOR_CONC_0;
	substate = SUBSTATE_CURSOR;
	row_offset = 0;

	sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
}

void ConcTable::okay(Sctp* sctp) {
	switch (substate) {
		case SUBSTATE_CURSOR: {
			if (cursor <= CURSOR_CONC_3) {
				substate = SUBSTATE_CONCENTRATION;
			}
			else if (cursor <= CURSOR_ABSORBANCE_3) {
				sctp->point_sel = cursor - 4 + row_offset;
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
					if (sctp->blank_take->readout != NULL) free(sctp->blank_take->readout);
					free(sctp->blank_take);
					sctp->blank_take = NULL;
				}

                // free sctp->curve buffers
                free(sctp->curve.filename);
                sctp->curve.filename = NULL;
                free(sctp->curve.absorbance);
                sctp->curve.absorbance = NULL;
                free(sctp->curve.concentration);
                sctp->curve.concentration = NULL;

                sctp->setState(ConcCurves::getInstance());
			}
			break;
		}
		case SUBSTATE_CONCENTRATION: {
			substate = SUBSTATE_CURSOR;
			if ( (sctp->curve.concentration[row_offset + cursor] != 0) && (sctp->curve.absorbance[row_offset + cursor] != 0) ) {
				sctp->curve.points++;
			}
			else if ((sctp->curve.concentration[row_offset + cursor] == 0) && (sctp->curve.absorbance[row_offset + cursor] == 0)) {
				sctp->curve.points--;
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
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
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
			sctp_lcd_conc_table_cursor(cursor, row_offset, sctp->curve);
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