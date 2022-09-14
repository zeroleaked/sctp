#include <esp_log.h>

#include "sctp_lcd.h"
#include "menu_state.h"
#include "spec_blank_state.h"

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
                    command_t command = SPECTRUM_BLANK;
                    xTaskCreatePinnedToCore(sctp->sampleSpectrumBlankWrapper, "command handler", 2048, sctp, 3, &sctp->task_command_handler, 1);
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
                    vTaskDelete(sctp->task_command_handler);
                    free(sctp->blank_sample.readout);
                    sctp->blank_sample.readout = NULL;
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