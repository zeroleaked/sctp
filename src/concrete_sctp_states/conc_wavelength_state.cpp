#include "sctp_lcd.h"
#include "menu_state.h"
#include "conc_standards_state.h"
#include "conc_wavelength_state.h"

#define SUBSTATE_WL 0
#define SUBSTATE_CURSOR 1

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
    substate = SUBSTATE_WL;
    cursor = CURSOR_WL;
	sctp_lcd_wavelength(wavelength);
}

void ConcWavelength::okay(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        // Finish wavelength change
        substate = SUBSTATE_CURSOR;
    } else { // substate = SUBSTATE_CURSOR
        // Next state
        switch (cursor) {
            case CURSOR_WL: {
                substate = SUBSTATE_WL;
                break;
            }
            case CURSOR_NEXT: {
                sctp->wavelength = wavelength;
                // next state
                sctp->setState(ConcStandards::getInstance()); 
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
    if (substate == SUBSTATE_WL) {
        wavelength = wavelength + 2;
        if (wavelength > 700) wavelength = 400;
        sctp_lcd_wavelength_number(wavelength);
    } else { // substate = SUBSTATE_CURSOR
        if (cursor == CURSOR_WL) cursor = CURSOR_NEXT;
        else cursor = CURSOR_WL;
    }
}

void ConcWavelength::arrowDown(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        wavelength = wavelength - 2;
        if (wavelength < 400) wavelength = 700;
        sctp_lcd_wavelength_number(wavelength);
    } else { // substate = SUBSTATE_CURSOR
        arrowUp(sctp);
    }
}

void ConcWavelength::arrowRight(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        arrowUp(sctp);
    } else { // substate = SUBSTATE_CURSOR
        if (cursor == CURSOR_NEXT) cursor = CURSOR_BACK;
        else if (cursor == CURSOR_BACK) cursor = CURSOR_NEXT;
    }
}

void ConcWavelength::arrowLeft(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        arrowDown(sctp);
    } else { // substate = SUBSTATE_CURSOR
        arrowRight(sctp);
    }
}

SctpState& ConcWavelength::getInstance()
{
	static ConcWavelength singleton;
	return singleton;
}