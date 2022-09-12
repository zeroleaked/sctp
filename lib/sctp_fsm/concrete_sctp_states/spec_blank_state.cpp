#include "sctp_lcd.h"
#include "menu_state.h"
#include "spec_blank_state.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

// dummy LCD
void sctp_lcd_spec_blank(uint8_t cursor){};
void sctp_lcd_spec_blank_clear(uint8_t cursor){};
void sctp_lcd_spec_blank_sampling(){};

// dummy COMMAND
void sctp_spec_blank() {};

void SpecBlank::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = 0;
	sctp_lcd_spec_blank(cursor);
}

void SpecBlank::okay(Sctp* sctp)
{
	// Low -> Medium
    if (substate == SUBSTATE_WAITING) {
        switch (cursor) {
            case CURSOR_NEXT: {
                substate = SUBSTATE_SAMPLING;
                sctp_lcd_spec_blank_sampling();
                sctp_spec_blank();
                break;
            }
            case CURSOR_CANCEL: {
                sctp->setState(Menu::getInstance());
                break;
            }
        }
    }
    else { // substate == SUBSTATE_SAMPLING
        switch (cursor) {
            case CURSOR_CANCEL: {
                // todo check memory leak

                substate = SUBSTATE_WAITING;
                sctp_lcd_spec_blank(cursor);
                break;
            }
        }
    }
}

void SpecBlank::arrowLeft(Sctp* sctp)
{
    if (substate == SUBSTATE_WAITING) {
        sctp_lcd_spec_blank_clear(cursor);
        if (cursor == CURSOR_NEXT) cursor = CURSOR_CANCEL;
        else if (cursor == CURSOR_CANCEL) cursor = CURSOR_NEXT;
        sctp_lcd_spec_blank(cursor);
    } else { // substate == SUBSTATE_SAMPLING
        cursor = CURSOR_CANCEL;
    }
}

SctpState& SpecBlank::getInstance()
{
	static SpecBlank singleton;
	return singleton;
}