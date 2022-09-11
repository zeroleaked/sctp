#include "sctp_lcd.h"
#include "conc_wavelength_state.h"
#include "conc_standards_state.h"

#define CURSOR_SAVE 0
#define CURSOR_RESAMPLE 1
#define CURSOR_REBLANK 2
#define CURSOR_MENU 3

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
// 	switch (cursor) {
//         case CURSOR_SAVE: {
//             sctp_save_spec();
// 			// lcd??
// 			sctp_lcd_spec_result_saving();
//             break;
//         } 
// 		case CURSOR_RESAMPLE: {
// 			sctp->setState(SpecSample::getInstance());
// 			break;
// 		}
// 		case CURSOR_REBLANK: {
// 			sctp->setState(SpecBlank::getInstance());
// 			break;
// 		}
// 		case CURSOR_MENU: {
// 			sctp->setState(Menu::getInstance());
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