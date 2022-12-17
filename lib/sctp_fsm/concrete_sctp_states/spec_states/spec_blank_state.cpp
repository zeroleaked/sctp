#include <esp_log.h>

#include "spec_blank_state.h"
#include "../menu_state.h"
#include "spec_sample_state.h"
#include "sctp_lcd.h"
#include "sctp_sensor.h"

#define CURSOR_NEXT 0
#define CURSOR_CHECK 1
#define CURSOR_CANCEL 2
#define CURSOR_NULL 3

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "spec_blank_state";

typedef struct {
    QueueHandle_t report_queue;
    calibration_t * calibration;
    blank_take_t * blank_take;
    uint8_t * percentage;
} taskParam_t;

void SpecBlank::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_CHECK;
    check_result = (uint16_t*)malloc(sizeof(uint16_t));
    *check_result = 0;
    percentage = (uint8_t*)malloc(sizeof(uint8_t));
    *percentage = 0;
	sctp_lcd_spec_blank_waiting(cursor, *check_result);
}

static void takeSpectrumBlank(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;
    uint8_t * percentage = ((taskParam_t *) pvParameters)->percentage;

	assert(blank_take->readout != NULL);
	blank_take->exposure[0] = 7800;
	blank_take->gain = 1;
	esp_err_t report = sctp_sensor_spectrum_blank(calibration, blank_take, percentage);
	
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
                    sctp_lcd_spec_blank_clear(cursor);
                    sctp_lcd_spec_blank_waiting(cursor, *check_result);
                    cursor = CURSOR_NULL;
                    sctp_lcd_spec_blank_sampling(cursor, *percentage);

	                report_queue = xQueueCreate(1, sizeof(esp_err_t));
                    substate = SUBSTATE_SAMPLING;
                    
                    assert(sctp->blank_take == NULL);
                    sctp->blank_take = (blank_take_t *) malloc (sizeof(blank_take_t));
	                sctp->blank_take->readout = (float *) malloc(sizeof(float) * sctp->calibration.length);
	                sctp->blank_take->exposure = (uint16_t *) malloc(sizeof(uint16_t) * sctp->calibration.length);
                    taskParam = malloc (sizeof(taskParam_t));
                	((taskParam_t *) taskParam)->report_queue = report_queue;
                	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
                	((taskParam_t *) taskParam)->blank_take = sctp->blank_take;
                	((taskParam_t *) taskParam)->percentage = percentage;
                    
                    xTaskCreatePinnedToCore(takeSpectrumBlank, "takeSpectrumBlank", 8192, taskParam, 4, &taskHandle, 1);
                    break;
                }
                case CURSOR_CHECK:
                {
                    sctp_sensor_check(&sctp->calibration, check_result);
                    sctp_lcd_spec_blank_waiting(cursor, *check_result);
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
                    // vTaskDelete(taskHandle);
                    // taskHandle = NULL;
                    // vQueueDelete(report_queue);
                    // report_queue = NULL;

                    // free(taskParam);
                    // taskParam = NULL;
                    // free(sctp->blank_take->readout);
                    // sctp->blank_take->readout = NULL;
                    // free(sctp->blank_take);
                    // sctp->blank_take = NULL;

                    // substate = SUBSTATE_WAITING;
	                // sctp_lcd_spec_blank_waiting(cursor);
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
                    cursor = CURSOR_CHECK;
                    break;
                }
                case CURSOR_CHECK: {
                    cursor = CURSOR_CANCEL;
                    break;
                }
                case CURSOR_CANCEL: {
                    cursor = CURSOR_NEXT;
                    break;
                }
            }
            sctp_lcd_spec_blank_waiting(cursor, *check_result);
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_NULL: {
                    cursor = CURSOR_CANCEL;
	                sctp_lcd_spec_blank_sampling(cursor, *percentage);
                    break;
                }
                case CURSOR_CANCEL: {
                    sctp_lcd_spec_blank_sampling(cursor, *percentage);
                }
            }
        }
    }
}

void SpecBlank::arrowRight(Sctp* sctp)
{
    sctp_lcd_spec_blank_clear(cursor);
    if(substate == SUBSTATE_WAITING) {
        switch (cursor) {
            case CURSOR_NEXT: {
                cursor = CURSOR_CANCEL;
                break;
            }
            case CURSOR_CHECK: {
                cursor = CURSOR_NEXT;
                break;
            }
            case CURSOR_CANCEL: {
                cursor = CURSOR_CHECK;
                break;
            }
        }
        sctp_lcd_spec_blank_waiting(cursor, *check_result);
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
	    } else {
           sctp_lcd_spec_blank_sampling_percentage(*percentage);
        }
    }
}

void SpecBlank::exit(Sctp *sctp)
{
    free(check_result);
    free(percentage);
}

SctpState& SpecBlank::getInstance()
{
	static SpecBlank singleton;
	return singleton;
}