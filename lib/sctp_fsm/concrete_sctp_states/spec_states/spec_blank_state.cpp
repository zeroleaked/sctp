#include <esp_log.h>

#include "spec_blank_state.h"
#include "../menu_state.h"
#include "spec_sample_state.h"
#include "sctp_lcd.h"
#include "sctp_sensor.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1
#define CURSOR_NULL 2

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "spec_blank_state";

typedef struct {
    QueueHandle_t report_queue;
    calibration_t * calibration;
    blank_take_t * blank_take;
    float * sample_take;
} taskParam_t;

void SpecBlank::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_NEXT;
	sctp_lcd_spec_blank_waiting(cursor);
}

static void takeSpectrumBlank(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;

	assert(blank_take->readout != NULL);
	blank_take->exposure = 10;
	blank_take->gain = 1;
	esp_err_t report = sctp_sensor_spectrum_blank(calibration, blank_take);
	
    QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	vTaskDelete( NULL );

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

	                report_queue = xQueueCreate(1, sizeof(esp_err_t));
                    assert(sctp->blank_take == NULL);
                    sctp->blank_take = (blank_take_t *) malloc (sizeof(blank_take_t));
	                sctp->blank_take->readout = (float *) malloc(sizeof(float) * sctp->calibration.length);
                    taskParam = malloc (sizeof(taskParam_t));
                	((taskParam_t *) taskParam)->report_queue = report_queue;
                	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
                	((taskParam_t *) taskParam)->blank_take = sctp->blank_take;
                    
                    xTaskCreatePinnedToCore(takeSpectrumBlank, "takeSpectrumBlank", 2048, taskParam, 4, &taskHandle, 1);
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
                    vTaskDelete(taskHandle);
                    taskHandle = NULL;
                    vQueueDelete(report_queue);
                    report_queue = NULL;

                    free(taskParam);
                    taskParam = NULL;
                    free(sctp->blank_take->readout);
                    sctp->blank_take->readout = NULL;
                    free(sctp->blank_take);
                    sctp->blank_take = NULL;

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

void SpecBlank::refreshLcd(Sctp* sctp, command_t command) {
    // if (command == SPECTRUM_BLANK) {
	// 	sctp->setState(SpecSample::getInstance());
    // }


    if (substate == SUBSTATE_SAMPLING) {
        esp_err_t report;
        if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
            if (report == ESP_OK) {
                free(taskParam);
                taskParam = NULL;
                vQueueDelete(report_queue);
                report_queue = NULL;
                sctp->setState(SpecSample::getInstance());
            }
	    }
    }
}

SctpState& SpecBlank::getInstance()
{
	static SpecBlank singleton;
	return singleton;
}