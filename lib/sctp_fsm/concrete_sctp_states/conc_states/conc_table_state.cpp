#include <esp_log.h>

#include "conc_table_state.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define SUBSTATE_LOADING 0
#define SUBSTATE_CURSORING 1
#define SUBSTATE_CONCENTRATION 2

#define CURSOR_NULL 0
#define CURSOR

#define MAX_POINTS 15

static const char TAG[] = "conc_table_state";

typedef struct {
	QueueHandle_t report_queue;
	curve_t * curve;
} taskParam_t;

static void loadConcFloats(void * pvParameters) {
	curve_t * curve = ((taskParam_t *) pvParameters)->curve;
	QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;

	esp_err_t report = sctp_flash_load_curve_floats(curve);
	
	assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	ESP_LOGI(TAG, "loadConcFloats() sended to queue");
	vTaskDelete( NULL );
}

void ConcTable::enter(Sctp * sctp) {
	sctp_lcd_clear();

	cursor = CURSOR_NULL;
	substate = SUBSTATE_LOADING;

	sctp_lcd_conc_table_opening(cursor);

	sctp->curve.absorbance = (float *) malloc( MAX_POINTS * sizeof(float) );
	sctp->curve.concentration = (float *) malloc( MAX_POINTS * sizeof(float) );

	report_queue = xQueueCreate(1, sizeof(esp_err_t));

	// packing param
	taskParam = (taskParam_t *) malloc (sizeof(taskParam_t));
	((taskParam_t *) taskParam)->curve = &sctp->curve;
	((taskParam_t *) taskParam)->report_queue = report_queue;
    xTaskCreatePinnedToCore(loadConcFloats, "loadConcFloats", 2048, taskParam, 4, &taskHandle, 1);   
}

void ConcTable::refreshLcd(Sctp* sctp, command_t command) {
	if (substate == SUBSTATE_LOADING) {
		esp_err_t report;
		if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
			if (report == ESP_OK) {
				free(taskParam);
				taskParam = NULL;
				vQueueDelete(report_queue);
				report_queue = NULL;

				substate = SUBSTATE_CURSORING;
				// sctp_lcd_conc_curves_list(cursor, sctp->curve);
			}
		}
	}
};

SctpState& ConcTable::getInstance()
{
	static ConcTable singleton;
	return singleton;
}