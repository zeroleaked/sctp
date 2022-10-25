#include <esp_log.h>

#include "conc_save_state.h"
#include "conc_table_state.h"
#include "sctp_common_types.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_SAVING 1
#define SUBSTATE_WAITING 2

static const char * TAG = "conc_save_state";

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

void ConcSave::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;

	// report_queue = xQueueCreate(1, sizeof(esp_err_t));
    substate = SUBSTATE_SAVING;

	sctp_lcd_spec_save_saving();
	sctp_flash_save_curve(sctp->curve);

	sctp_lcd_spec_save_finish(sctp->curve.filename);
	substate = SUBSTATE_WAITING;
}

void ConcSave::arrowDown(Sctp* sctp)
{
	if (substate == SUBSTATE_WAITING) {
		cursor = CURSOR_OK;
		sctp_lcd_spec_save_finish_cursor(cursor);
	}
}

void ConcSave::okay(Sctp* sctp)
{
	if (cursor == CURSOR_OK) {
        sctp->setState(ConcTable::getInstance());
	}
}

SctpState& ConcSave::getInstance()
{
	static ConcSave singleton;
	return singleton;
}