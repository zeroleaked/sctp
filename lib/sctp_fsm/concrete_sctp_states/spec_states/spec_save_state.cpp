#include <esp_log.h>

#include "spec_save_state.h"
#include "spec_result_state.h"
#include "sctp_common_types.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_SAVING 0
#define SUBSTATE_WAITING 1

typedef struct {
	QueueHandle_t report_queue;
	float * absorbance;
	float * wavelength;
	calibration_t * calibration;
} taskParam_t;

static const char * TAG = "spec_save_state";

static void saveSpectrum(void * pvParameters) {
    float * absorbance = ((taskParam_t *) pvParameters)->absorbance;
	assert(absorbance != NULL);
    float * wavelength = ((taskParam_t *) pvParameters)->wavelength;
	assert(wavelength != NULL);
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;
	assert(calibration != NULL);

	esp_err_t report = (sctp_flash_save_spectrum(absorbance, wavelength, calibration->length));

    QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "saveSpectrum() sended to queue");
	vTaskDelete( NULL );
}

void SpecSave::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;
	ESP_LOGI(TAG, "enter, saving...");

	report_queue = xQueueCreate(1, sizeof(esp_err_t));
    substate = SUBSTATE_SAVING;
	
	taskParam = malloc (sizeof(taskParam_t));
	((taskParam_t *) taskParam)->report_queue = report_queue;
	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
	((taskParam_t *) taskParam)->absorbance = sctp->absorbance;
	((taskParam_t *) taskParam)->wavelength = sctp->spectrum_wavelength;
    xTaskCreatePinnedToCore(saveSpectrum, "spectrum save", 2048, taskParam, 4, &taskHandle, 1);
	sctp_lcd_spec_save_saving();
}

void SpecSave::arrowDown(Sctp* sctp)
{
	if (substate == SUBSTATE_WAITING) {
		cursor = CURSOR_OK;
		sctp_lcd_spec_save_finish_cursor(cursor);
	}
}

void SpecSave::okay(Sctp* sctp)
{
	if (cursor == CURSOR_OK) {
        sctp->setState(SpecResult::getInstance());
	}
}

void SpecSave::refreshLcd(Sctp* sctp, command_t command)
{
    // if (command == SPECTRUM_SAVE) {
	// 	substate = SUBSTATE_WAITING;
	// 	sctp_lcd_spec_save_finish(sctp->saved_name);
    // }
	// else
	if (substate == SUBSTATE_SAVING) {
		esp_err_t report;
		if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
			free(taskParam);
			taskParam = NULL;
			vQueueDelete(report_queue);
			report_queue = NULL;
			substate = SUBSTATE_WAITING;
			sctp_lcd_spec_save_finish(sctp->saved_name);
		}	
	}
}

SctpState& SpecSave::getInstance()
{
	static SpecSave singleton;
	return singleton;
}