#include "sctp_lcd.h"
#include "menu_state.h"
#include "spec_blank_state.h"
#include "spec_sample_state.h"
#include "spec_result_state.h"

#define CURSOR_SAVE 0
#define CURSOR_RESAMPLE 1
#define CURSOR_REBLANK 2
#define CURSOR_MENU 3

// lcd dummies
void sctp_lcd_spec_result();
void sctp_lcd_spec_result_saving();

// command dummies
void sctp_save_spec();

void SpecResult::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = 0;
	sctp_lcd_spec_result();
	// Off -> Low
}

void SpecResult::okay(Sctp* sctp)
{
	switch (cursor) {
        case CURSOR_SAVE: {
            sctp_save_spec();
			// lcd??
			sctp_lcd_spec_result_saving();
            break;
        } 
		case CURSOR_RESAMPLE: {
			sctp->setState(SpecSample::getInstance());
			break;
		}
		case CURSOR_REBLANK: {
			sctp->setState(SpecBlank::getInstance());
			break;
		}
		case CURSOR_MENU: {
			sctp->setState(Menu::getInstance());
			break;
		}
	}
}

void SpecResult::arrowUp(Sctp* sctp)
{
	sctp_lcd_menu_clear(this->cursor);
	if (this->cursor == 0) this->cursor = 3;
    else this->cursor--;
	sctp_lcd_menu(this->cursor);
}

void SpecResult::arrowDown(Sctp* sctp)
{
	sctp_lcd_menu_clear(this->cursor);
	if (this->cursor == 3) this->cursor = 0;
	else this->cursor++;
	sctp_lcd_menu(this->cursor);
}

SctpState& SpecResult::getInstance()
{
	static SpecResult singleton;
	return singleton;
}