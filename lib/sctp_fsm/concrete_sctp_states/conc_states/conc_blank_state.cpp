#include <esp_log.h>

#include "conc_blank_state.h"
#include "conc_table_state.h"
#include "conc_sample_state.h"
#include "sctp_lcd.h"
#include "sctp_sensor.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1
#define CURSOR_NULL 2

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "conc_blank_state";

typedef struct {
    QueueHandle_t report_queue;
    calibration_t * calibration;
	uint16_t wavelength;
    blank_take_t * blank_take;
} taskParam_t;

void ConcBlank::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_NEXT;
    
	// state buffers
	taskParam = malloc (sizeof(taskParam_t));
	report_queue = xQueueCreate(1, sizeof(esp_err_t));

	sctp_lcd_conc_blank_waiting(cursor);
}

static void takeConcentrationBlank(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    uint16_t wavelength = ((taskParam_t *) pvParameters)->wavelength;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;

	assert(blank_take->readout != NULL);
	blank_take->exposure = 10;
	blank_take->gain = 1;
	esp_err_t report = sctp_sensor_concentration_blank(calibration, wavelength, blank_take);
	
    QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "takeConcentrationBlank() sended to queue");
	vTaskDelete( NULL );

}

void ConcBlank::okay(Sctp* sctp)
{
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    cursor = CURSOR_NULL;
                    sctp_lcd_conc_blank_sampling(cursor);

                    substate = SUBSTATE_SAMPLING;

                    assert(sctp->blank_take == NULL);
                    sctp->blank_take = (blank_take_t *) malloc (sizeof(blank_take_t));
	                sctp->blank_take->readout = (float *) malloc(sizeof(float));

                	((taskParam_t *) taskParam)->report_queue = report_queue;
                	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
                	((taskParam_t *) taskParam)->wavelength = sctp->curve.wavelength;
                	((taskParam_t *) taskParam)->blank_take = sctp->blank_take;
                    
                    xTaskCreatePinnedToCore(takeConcentrationBlank, "takeConcentrationBlank", 2048, taskParam, 4, &taskHandle, 1);
                    break;
                }
                case CURSOR_CANCEL: {
                    sctp->setState(ConcTable::getInstance());
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

                    free(sctp->blank_take->readout);
                    sctp->blank_take->readout = NULL;
                    free(sctp->blank_take);
                    sctp->blank_take = NULL;

                    substate = SUBSTATE_WAITING;
	                sctp_lcd_conc_blank_waiting(cursor);
                    break;
                }
            }
			break;
        }
    }
}

void ConcBlank::arrowLeft(Sctp* sctp)
{
    sctp_lcd_conc_blank_clear(cursor);
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
	        sctp_lcd_conc_blank_waiting(cursor);
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_NULL: {
                    cursor = CURSOR_CANCEL;
                    break;
                }
            }
	        sctp_lcd_conc_blank_sampling(cursor);
			break;
        }
    }
}

void ConcBlank::refreshLcd(Sctp* sctp, command_t command) {
    // if (command == SPECTRUM_BLANK) {
	// 	sctp->setState(SpecSample::getInstance());
    // }


    if (substate == SUBSTATE_SAMPLING) {
        esp_err_t report;
        if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
            if (report == ESP_OK) {
                sctp->setState(ConcSample::getInstance());
            }
	    }
    }
}

void ConcBlank::exit(Sctp * sctp) {
	// free state buffers
	free(taskParam);
	taskParam = NULL;
	vQueueDelete(report_queue);
	report_queue = NULL;
}

SctpState& ConcBlank::getInstance()
{
	static ConcBlank singleton;
	return singleton;
}