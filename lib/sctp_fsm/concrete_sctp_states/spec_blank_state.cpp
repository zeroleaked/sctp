#include <esp_log.h>

#include "spec_blank_state.h"
#include "sctp_lcd.h"
#include "menu_state.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1
#define CURSOR_NULL 2

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "spec_blank_state";

void SpecBlank::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_NEXT;
	sctp_lcd_spec_blank_waiting(cursor);
}

void SpecBlank::okay(Sctp* sctp)
{
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    substate = SUBSTATE_SAMPLING;
                    cursor = CURSOR_NULL;
                    sctp_lcd_spec_blank_sampling(cursor);
                    xTaskCreatePinnedToCore(sctp->sampleSpectrumBlankWrapper, "command handler", 2048, sctp, 4, &sctp->task_spectrum_blank, 1);
                    break;
                }
                case CURSOR_CANCEL: {
                    sctp->setState(Menu::getInstance());
                    break;
                }
            }
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_CANCEL: {
                    vTaskDelete(sctp->task_spectrum_blank);
                    free(sctp->blank_take.readout);
                    sctp->blank_take.readout = NULL;
                    substate = SUBSTATE_WAITING;
	                sctp_lcd_spec_blank_waiting(cursor);
                    break;
                }
            }
        }
    }
}

void SpecBlank::arrowLeft(Sctp* sctp)
{
    sctp_lcd_spec_blank_clear(cursor);
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    cursor = CURSOR_CANCEL;
                    break;
                }
                case CURSOR_CANCEL: {
                    cursor = CURSOR_NEXT;
                    break;
                }
            }
	        sctp_lcd_spec_blank_waiting(cursor);
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_NULL: {
                    cursor = CURSOR_CANCEL;
	                sctp_lcd_spec_blank_sampling(cursor);
                    break;
                }
            }
        }
    }
}

SctpState& SpecBlank::getInstance()
{
	static SpecBlank singleton;
	return singleton;
}