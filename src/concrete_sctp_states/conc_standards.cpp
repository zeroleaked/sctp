#include "sctp_lcd.h"
#include "conc_wavelength_state.h"
#include "conc_standards_state.h"

#define CURSOR_STANDARD_0 0
#define CURSOR_STANDARD_1 1
#define CURSOR_STANDARD_2 2
#define CURSOR_STANDARD_3 3
#define CURSOR_STANDARD_4 4
#define CURSOR_NEXT_PAGE 5
#define CURSOR_PREV_PAGE 6
#define CURSOR_NEW 7
#define CURSOR_BACK 8

// lcd dummies
void sctp_lcd_conc_standards(uint8_t cursor) {};
void sctp_lcd_spec_standards_clear(uint8_t cursor) {};

// command dummies
void sctp_load_conc_standards() {};


void ConcStandards::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = 0;
    sctp_load_conc_standards();

	sctp_lcd_conc_standards(cursor);
}

// void ConcStandards::okay(Sctp* sctp)
// {
//     if (cursor < 5) {
//         sctp->standard = standards[page*5 + cursor];
//     }
// 	switch (cursor) {
// 		case CURSOR_NEW: {
// 			sctp->setState(SpecBlank::getInstance());
// 			break;
// 		}
// 		case CURSOR_BACK: {
// 			sctp->setState(ConcWavelength::getInstance());
// 			break;
// 		}
// 	}
// }

// void ConcStandards::arrowUp(Sctp* sctp)
// {
// 	sctp_lcd_menu_clear(this->cursor);
// 	if (this->cursor == 0) this->cursor = 3;
//     else this->cursor--;
// 	sctp_lcd_menu(this->cursor);
// }

// void ConcStandards::arrowDown(Sctp* sctp)
// {
// 	sctp_lcd_menu_clear(this->cursor);
// 	if (this->cursor == 3) this->cursor = 0;
// 	else this->cursor++;
// 	sctp_lcd_menu(this->cursor);
// }

SctpState& ConcStandards::getInstance()
{
	static ConcStandards singleton;
	return singleton;
}