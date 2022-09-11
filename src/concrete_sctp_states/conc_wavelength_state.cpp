#include "sctp_lcd.h"
#include "conc_wavelength_state.h"
#include "menu_state.h"

#define BUTTON_SET_WL 0
#define BUTTON_SET_CURSOR 1

#define CURSOR_WL 0
#define CURSOR_NEXT 1
#define CURSOR_BACK 2

// lcd dummies. declare this later in sctp_lcd.h
void sctp_lcd_wavelength(uint16_t wavelength) {}
void sctp_lcd_wavelength_number(uint16_t wavelength) {}

void ConcWavelength::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	wavelength = 550;
    button_set = BUTTON_SET_WL;
    cursor = CURSOR_WL;
	sctp_lcd_wavelength(wavelength);
}

void ConcWavelength::okay(Sctp* sctp)
{
    if (button_set == BUTTON_SET_WL) {
        // Finish wavelength change
        button_set = BUTTON_SET_CURSOR;
    } else { // button_set = BUTTON_SET_CURSOR
        // Next state
        switch (cursor) {
            case CURSOR_WL: {
                button_set = BUTTON_SET_WL;
                break;
            }
            case CURSOR_NEXT: {
                sctp->wavelength = wavelength;
                // next state
                // sctp->setState(Menu::getInstance()); 
                break;
            }
            case CURSOR_BACK: {
                sctp->setState(Menu::getInstance());
                break;
            }
        }
    }
}

void ConcWavelength::arrowUp(Sctp* sctp)
{
    if (button_set == BUTTON_SET_WL) {
        wavelength = wavelength + 2;
        if (wavelength > 700) wavelength = 400;
        sctp_lcd_wavelength_number(wavelength);
    } else { // button_set = BUTTON_SET_CURSOR
        if (cursor == CURSOR_WL) cursor = CURSOR_NEXT;
        else cursor = CURSOR_WL;
    }
}

void ConcWavelength::arrowDown(Sctp* sctp)
{
    if (button_set == BUTTON_SET_WL) {
        wavelength = wavelength - 2;
        if (wavelength < 400) wavelength = 700;
        sctp_lcd_wavelength_number(wavelength);
    } else { // button_set = BUTTON_SET_CURSOR
        arrowUp(sctp);
    }
}

void ConcWavelength::arrowRight(Sctp* sctp)
{
    if (button_set == BUTTON_SET_WL) {
        arrowUp(sctp);
    } else { // button_set = BUTTON_SET_CURSOR
        if (cursor == CURSOR_NEXT) cursor = CURSOR_BACK;
        else if (cursor == CURSOR_BACK) cursor = CURSOR_NEXT;
    }
}

void ConcWavelength::arrowLeft(Sctp* sctp)
{
    if (button_set == BUTTON_SET_WL) {
        arrowDown(sctp);
    } else { // button_set = BUTTON_SET_CURSOR
        arrowRight(sctp);
    }
}

SctpState& ConcWavelength::getInstance()
{
	static ConcWavelength singleton;
	return singleton;
}