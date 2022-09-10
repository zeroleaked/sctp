#include "sctp_lcd.h"
#include "menu_state.h"
// #include "wavelength_state.h"

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
		case 1: {
			// sctp->setState(Wavelength::getInstance());
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