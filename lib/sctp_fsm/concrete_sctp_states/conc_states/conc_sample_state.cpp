#include <esp_log.h>
#include <math.h>

#include "conc_sample_state.h"
#include "conc_table_state.h"
#include "sctp_lcd.h"
#include "sctp_sensor.h"
#include "sctp_flash.h"

#define CURSOR_NEXT 0
#define CURSOR_CANCEL 1
#define CURSOR_NULL 2

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "conc_sample_state";

typedef struct {
    QueueHandle_t report_queue;
    calibration_t * calibration; // input
	uint16_t wavelength; // input
    blank_take_t * blank_take; // input
	float * absorbance; // output
} taskParam_t;

void ConcSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_NEXT;
    
	// state buffers
	taskParam = malloc (sizeof(taskParam_t));
	report_queue = xQueueCreate(1, sizeof(esp_err_t));

	sctp_lcd_conc_sample_waiting(cursor);
}

static void takeConcentrationSample(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    uint16_t wavelength = ((taskParam_t *) pvParameters)->wavelength;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;
    float * absorbance = ((taskParam_t *) pvParameters)->absorbance;

	float sample_take;
	esp_err_t report = sctp_sensor_concentration_sample(calibration, wavelength, blank_take, &sample_take);
	float transmission = (*absorbance) / (*(blank_take->readout));
	*absorbance = -log10(transmission);

    QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "takeConcentrationSample() sended to queue");
	vTaskDelete( NULL );

}

void ConcSample::okay(Sctp* sctp)
{
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    cursor = CURSOR_NULL;
                    sctp_lcd_conc_sample_sampling(cursor);

                    substate = SUBSTATE_SAMPLING;

                	((taskParam_t *) taskParam)->report_queue = report_queue;
                	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
                	((taskParam_t *) taskParam)->wavelength = sctp->curve.wavelength;
                	((taskParam_t *) taskParam)->blank_take = sctp->blank_take;
                	((taskParam_t *) taskParam)->absorbance = &absorbance;
                    
                    xTaskCreatePinnedToCore(takeConcentrationSample, "takeConcentrationSample", 2048, taskParam, 4, &taskHandle, 1);
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

                    substate = SUBSTATE_WAITING;
	                sctp_lcd_conc_sample_waiting(cursor);
                    break;
                }
            }
			break;
        }
    }
}

void ConcSample::arrowLeft(Sctp* sctp)
{
    sctp_lcd_conc_sample_clear(cursor);
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
	        sctp_lcd_conc_sample_waiting(cursor);
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_NULL: {
                    cursor = CURSOR_CANCEL;
                    break;
                }
            }
	        sctp_lcd_conc_sample_sampling(cursor);
			break;
        }
    }
}

void ConcSample::refreshLcd(Sctp* sctp, command_t command) {
    // if (command == SPECTRUM_BLANK) {
	// 	sctp->setState(SpecSample::getInstance());
    // }


    if (substate == SUBSTATE_SAMPLING) {
        esp_err_t report;
        if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
            if (report == ESP_OK) {
				sctp->curve.absorbance[sctp->point_sel] = absorbance;
                sctp_flash_nvs_save_curve(&sctp->curve);
                sctp->setState(ConcTable::getInstance());
            }
	    }
    }
}

void ConcSample::exit(Sctp * sctp) {
	// free state buffers
	free(taskParam);
	taskParam = NULL;
	vQueueDelete(report_queue);
	report_queue = NULL;
}

SctpState& ConcSample::getInstance()
{
	static ConcSample singleton;
	return singleton;
}