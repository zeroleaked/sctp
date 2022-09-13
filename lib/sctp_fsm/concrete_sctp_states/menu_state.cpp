#include "menu_state.h"
#include "spec_blank_state.h"
#include "conc_curves_state.h"
#include "history_list_state.h"
#include "settings_state.h"
#include "sctp_lcd.h"

#define CURSOR_SPECTRUM 0
#define CURSOR_QUANTIZATION 1
#define CURSOR_HISTORY 2
#define CURSOR_SETTINGS 3

void Menu::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	this->cursor = 0;
	sctp_lcd_menu(this->cursor);
	// Off -> Low
}

void Menu::okay(Sctp* sctp)
{
	// Low -> Medium
	switch (this->cursor) {
        case CURSOR_SPECTRUM: {
            sctp->setState(SpecBlank::getInstance());
            break;
        } 
		case CURSOR_QUANTIZATION: {
			sctp->setState(ConcCurves::getInstance());
			break;
		}
		case CURSOR_HISTORY: {
			sctp->setState(HistoryList::getInstance());
			break;
		}
		case CURSOR_SETTINGS: {
			sctp->setState(Settings::getInstance());
			break;
		}
	}
}

void Menu::arrowUp(Sctp* sctp)
{
	sctp_lcd_menu_clear(this->cursor);
	if (this->cursor == 0) this->cursor = 3;
    else this->cursor--;
	sctp_lcd_menu(this->cursor);
}

void Menu::arrowDown(Sctp* sctp)
{
	sctp_lcd_menu_clear(this->cursor);
	if (this->cursor == 3) this->cursor = 0;
	else this->cursor++;
	sctp_lcd_menu(this->cursor);
}

SctpState& Menu::getInstance()
{
	static Menu singleton;
	return singleton;
}