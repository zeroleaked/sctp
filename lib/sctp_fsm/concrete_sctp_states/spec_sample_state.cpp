#include <esp_log.h>

#include "spec_sample_state.h"
#include "spec_blank_state.h"
#include "spec_result_state.h"
#include "sctp_lcd.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1
#define CURSOR_NULL 2

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "spec_blank_state";

void SpecSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_NEXT;
	sctp_lcd_spec_sample_waiting(cursor);
}

void SpecSample::okay(Sctp* sctp)
{
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    substate = SUBSTATE_SAMPLING;
                    cursor = CURSOR_NULL;
                    sctp_lcd_spec_blank_sampling(cursor);
                    xTaskCreatePinnedToCore(sctp->sampleSpectrumSampleWrapper, "spectrum sample", 2048, sctp, 4, &sctp->task_spectrum_sample, 1);
                    break;
                }
                case CURSOR_CANCEL: {
                    sctp->setState(SpecBlank::getInstance());
                    break;
                }
            }
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_CANCEL: {
                    vTaskDelete(sctp->task_spectrum_sample);
                    free(sctp->sample_take);
                    sctp->sample_take = NULL;
                    substate = SUBSTATE_WAITING;
	                sctp_lcd_spec_sample_waiting(cursor);
                    break;
                }
            }
        }
    }
}

void SpecSample::arrowLeft(Sctp* sctp)
{
	sctp_lcd_spec_sample_clear(cursor);
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
	        sctp_lcd_spec_sample_waiting(cursor);
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_NULL: {
                    cursor = CURSOR_CANCEL;
	                sctp_lcd_spec_sample_sampling(cursor);
                    break;
                }
            }
        }
    }
}

void SpecSample::refreshLcd(Sctp* sctp) {
    command_t command;
    assert(xQueueReceive(sctp->lcd_refresh_queue, &command, 0) == pdTRUE); // already peeked
    if (command == SPECTRUM_SAMPLE) { // sample taken
		sctp->setState(SpecResult::getInstance());
    }
}

SctpState& SpecSample::getInstance()
{
	static SpecSample singleton;
	return singleton;
}