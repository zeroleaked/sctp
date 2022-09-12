#include "sctp_lcd.h"
#include "conc_wavelength_state.h"
#include "conc_curves_state.h"

#define CURSOR_CURVE_0 0
#define CURSOR_CURVE_1 1
#define CURSOR_CURVE_2 2
#define CURSOR_CURVE_3 3
#define CURSOR_CURVE_4 4
#define CURSOR_NEXT_PAGE 5
#define CURSOR_PREV_PAGE 6
#define CURSOR_NEW 7
#define CURSOR_BACK 8

// lcd dummies
void sctp_lcd_conc_curves(uint8_t cursor, uint8_t page, uint8_t length) {};
void sctp_lcd_spec_curves_clear(uint8_t cursor) {};

// command dummies
void sctp_load_conc_curves() {};


void ConcCurves::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = 0;
	page = 0;
    sctp_load_conc_curves();
	if ( curve_length >= 5) sctp_lcd_conc_curves(cursor, page, curve_length);
	else  sctp_lcd_conc_curves(cursor, page, 5);
}

void ConcCurves::okay(Sctp* sctp)
{
// ( (curve_length - (5*page) ) / 5 != 0)


    if (cursor < 5) {
		// load curve, free all the other memories
        sctp->curve = &(curves[page*5 + cursor]);
    }
	// switch (cursor) {
	// 	case CURSOR_NEW: {
	// 		sctp->setState(SpecBlank::getInstance());
	// 		break;
	// 	}
	// 	case CURSOR_BACK: {
	// 		sctp->setState(ConcWavelength::getInstance());
	// 		break;
	// 	}
	// }
}

// void ConcCurves::arrowUp(Sctp* sctp)
// {
// 	sctp_lcd_menu_clear(this->cursor);
// 	if (this->cursor == 0) this->cursor = 3;
//     else this->cursor--;
// 	sctp_lcd_menu(this->cursor);
// }

// void ConcCurves::arrowDown(Sctp* sctp)
// {
// 	sctp_lcd_menu_clear(this->cursor);
// 	if (this->cursor == 3) this->cursor = 0;
// 	else this->cursor++;
// 	sctp_lcd_menu(this->cursor);
// }

SctpState& ConcCurves::getInstance()
{
	static ConcCurves singleton;
	return singleton;
}