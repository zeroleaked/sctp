#include "sctp_lcd.h"
#include "spec_blank_state.h"
#include "spec_sample_state.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

// dummy LCD
void sctp_lcd_spec_sample(uint8_t cursor){};
void sctp_lcd_spec_sample_clear(uint8_t cursor){};
void sctp_lcd_spec_sample_sampling(){};

// dummy COMMAND
void sctp_spec_sample();

void SpecSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = 0;
	sctp_lcd_spec_sample(cursor);
}

void SpecSample::okay(Sctp* sctp)
{
	// Low -> Medium
    if (substate == SUBSTATE_WAITING) {
        switch (cursor) {
            case CURSOR_NEXT: {
                substate = SUBSTATE_SAMPLING;
                sctp_lcd_spec_sample_sampling();
                sctp_spec_sample();
                break;
            }
            case CURSOR_CANCEL: {
                sctp->setState(SpecBlank::getInstance());
                break;
            }
        }
    }
    else { // substate == SUBSTATE_SAMPLING
        switch (cursor) {
            case CURSOR_CANCEL: {
                // todo check memory leak

                substate = SUBSTATE_WAITING;
                sctp_lcd_spec_sample(cursor);
                break;
            }
        }
    }
}

void SpecSample::arrowLeft(Sctp* sctp)
{
    if (substate == SUBSTATE_WAITING) {
        sctp_lcd_spec_sample_clear(cursor);
        if (cursor == CURSOR_NEXT) cursor = CURSOR_CANCEL;
        else if (cursor == CURSOR_CANCEL) cursor = CURSOR_NEXT;
        sctp_lcd_spec_sample(cursor);
    } else { // substate == SUBSTATE_SAMPLING
        cursor = CURSOR_CANCEL;
    }
}

SctpState& SpecSample::getInstance()
{
	static SpecSample singleton;
	return singleton;
}