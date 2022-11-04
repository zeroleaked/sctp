#include <esp_log.h>
#include <math.h>

#include "spec_sample_state.h"
#include "spec_blank_state.h"
#include "spec_result_state.h"
#include "sctp_lcd.h"
#include "sctp_sensor.h"

#define CURSOR_NEXT 0
#define CURSOR_CHECK 1
#define CURSOR_CANCEL 2
#define CURSOR_NULL 3

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "spec_sample_state";

typedef struct {
    QueueHandle_t report_queue;
    calibration_t * calibration;
    blank_take_t * blank_take;
    float * sample_take;
    float * absorbance;
} taskParam_t;

void SpecSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_CHECK;
    check_result = (uint16_t *)malloc(sizeof(uint16_t));
    *check_result = 0;
    sctp_lcd_spec_sample_waiting(cursor, *check_result);
}

static void takeSpectrumSample(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    float * sample_take = ((taskParam_t *) pvParameters)->sample_take;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;

	esp_err_t report = sctp_sensor_spectrum_sample(calibration, blank_take, sample_take);

    float * absorbance = ((taskParam_t *) pvParameters)->absorbance;

	// castings
	float * blank_buffer = blank_take->readout;
	float * sample_buffer = sample_take;
	for (int i=0; i < calibration->length; i++) {
		float transmission = sample_buffer[i]/blank_buffer[i];
		absorbance[i] = -log10(transmission);
	}
	
    QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;
    assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	vTaskDelete( NULL );

}

void SpecSample::okay(Sctp* sctp)
{
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    sctp_lcd_spec_blank_clear(cursor);
                    sctp_lcd_spec_blank_waiting(cursor, *check_result);
                    cursor = CURSOR_NULL;
                    sctp_lcd_spec_sample_sampling(cursor);

	                report_queue = xQueueCreate(1, sizeof(esp_err_t));
                    substate = SUBSTATE_SAMPLING;
                    
                    assert(sctp->sample_take == NULL);
                    sctp->sample_take = (float *) malloc (sizeof(float) * sctp->calibration.length);
                    assert(sctp->absorbance == NULL);
                    sctp->absorbance = (float *) malloc (sizeof(float) * sctp->calibration.length);
                    taskParam = malloc (sizeof(taskParam_t));
                	((taskParam_t *) taskParam)->report_queue = report_queue;
                	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
                	((taskParam_t *) taskParam)->blank_take = sctp->blank_take;
                	((taskParam_t *) taskParam)->sample_take = sctp->sample_take;
                	((taskParam_t *) taskParam)->absorbance = sctp->absorbance;

                    xTaskCreatePinnedToCore(takeSpectrumSample, "takeSpectrumSample", 4096, taskParam, 4, &taskHandle, 1);
                    break;
                }
                case CURSOR_CHECK:
                {
                    sctp_sensor_check(&sctp->calibration, check_result);
                    sctp_lcd_spec_sample_waiting(cursor, *check_result);
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
                    vTaskDelete(taskHandle);
                    taskHandle = NULL;
                    vQueueDelete(report_queue);
                    report_queue = NULL;

                    free(sctp->sample_take);
                    sctp->sample_take = NULL;
                    free(sctp->absorbance);
                    sctp->absorbance = NULL;
                    free(taskParam);
                    taskParam = NULL;

                    substate = SUBSTATE_WAITING;
                    sctp_lcd_spec_sample_waiting(cursor, *check_result);
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
            sctp_lcd_spec_sample_waiting(cursor, *check_result);
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

void SpecSample::arrowRight(Sctp* sctp)
{
    sctp_lcd_spec_sample_clear(cursor);
    if(substate == SUBSTATE_WAITING) {
        switch (cursor)
        {
            case CURSOR_NEXT:
            {
                cursor = CURSOR_CANCEL;
                break;
            }
            case CURSOR_CHECK:
            {
                cursor = CURSOR_NEXT;
                break;
            }
            case CURSOR_CANCEL:
            {
                cursor = CURSOR_CHECK;
                break;
            }
        }
    sctp_lcd_spec_sample_waiting(cursor, *check_result);
    }
}

void SpecSample::refreshLcd(Sctp* sctp, command_t command) {
    // if (command == SPECTRUM_SAMPLE) { // sample taken
	// 	sctp->setState(SpecResult::getInstance());
    // }

    if (substate == SUBSTATE_SAMPLING) {
        esp_err_t report;
        if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
            if (report == ESP_OK) {
                free(taskParam);
                taskParam = NULL;
                vQueueDelete(report_queue);
                report_queue = NULL;
                sctp->setState(SpecResult::getInstance());
            }
        }
    }
}

void SpecSample::exit(Sctp *sctp)
{
    free(check_result);
}

SctpState& SpecSample::getInstance()
{
	static SpecSample singleton;
	return singleton;
}