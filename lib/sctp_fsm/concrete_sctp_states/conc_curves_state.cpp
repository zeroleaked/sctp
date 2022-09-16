#include <string.h>

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

void ConcCurves::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;
	substate = SUBSTATE_LOADING;

	sctp_lcd_conc_curves_opening(cursor);

    xTaskCreatePinnedToCore(sctp->loadConcCurveWrapper, "curve load", 2048, sctp, 4, NULL, 1);   
}

void ConcCurves::okay(Sctp* sctp)
{
	switch (substate) {
		case SUBSTATE_LOADING: {}
		case SUBSTATE_WAITING: {
			if (cursor <= CURSOR_CURVE_5) {
        		sctp->curve = curves[cursor];
				sctp->curve.filename = (char *) malloc( 20 * sizeof(char) );
				strcpy( sctp->curve.filename, curves[cursor].filename );
				for (int i=0; i<6; i++) {
					free(sctp->curves[i].filename);
				}
				sctp->setState(ConcTable::getInstance());
			}
			else if (cursor <= CURSOR_DEL_CURVE_5) {
				sctp->curves[cursor - 6].wavelength = 0;
				sctp->curves[cursor - 6].points = 0;

				if ( sctp->curves[cursor - 6].absorbance != NULL) {
					free(sctp->curves[cursor - 6].absorbance);
					sctp->curves[cursor - 6].absorbance = NULL;
				}
				
				if ( sctp->curves[cursor - 6].concentration != NULL) {
					free(sctp->curves[cursor - 6].concentration);
					sctp->curves[cursor - 6].concentration = NULL;
				}
				
    			// xTaskCreatePinnedToCore(sctp->delConcCurveWrapper, "curve delete", 2048, sctp, 4, NULL, 1);   
				sctp_lcd_conc_curves_list(cursor, curves);
			}
			else if (cursor == CURSOR_BACK) {
				for (int i=0; i<6; i++) {
					free(sctp->curves[i].filename);
				}
				sctp->setState(Menu::getInstance());
			}
		}
	}
}

void ConcCurves::arrowDown(Sctp* sctp)
{
	switch (substate) {
		case SUBSTATE_LOADING: {
			cursor = CURSOR_BACK;
			sctp_lcd_conc_curves_opening(cursor);
			break;
		}
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
		case SUBSTATE_LOADING: {
			cursor = CURSOR_BACK;
			sctp_lcd_conc_curves_opening(cursor);
			break;
		}
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
		case SUBSTATE_LOADING: {
			cursor = CURSOR_BACK;
			sctp_lcd_conc_curves_opening(cursor);
			break;
		}
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
	if (command == CURVES_LOAD) {
		substate = SUBSTATE_WAITING;
		sctp_lcd_spec_save_finish(sctp->saved_name);
	}

	
};


SctpState& ConcCurves::getInstance()
{
	static ConcCurves singleton;
	return singleton;
}