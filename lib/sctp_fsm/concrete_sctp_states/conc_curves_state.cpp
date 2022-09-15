#include "conc_curves_state.h"
#include "conc_wavelength_state.h"
#include "sctp_lcd.h"

#define CURSOR_CURVE_0 0
#define CURSOR_CURVE_1 1
#define CURSOR_CURVE_2 2
#define CURSOR_CURVE_3 3
#define CURSOR_CURVE_4 4
#define CURSOR_NEW 5
#define CURSOR_BACK 6
#define CURSOR_NULL 7

// command dummies
void sctp_load_conc_curves(curve_t * curves, uint8_t * curves_length) {};


void ConcCurves::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = 0;
	substate = CURSOR_NULL;
    // sctp_load_conc_curves(sctp->curves);

	sctp_lcd_conc_curves_opening(cursor);
}

void ConcCurves::okay(Sctp* sctp)
{
    if (cursor < 5) {
		// load curve, free all the other memories
        // sctp->curve = &(curves[cursor]);
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