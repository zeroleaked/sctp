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

static const float filter[] = {-1.54228316e-04, -1.50785197e-04, -1.47649846e-04, -1.44775525e-04,
                               -1.42110809e-04, -1.39599651e-04, -1.37181472e-04, -1.34791263e-04,
                               -1.32359696e-04, -1.29813259e-04, -1.27074397e-04, -1.24061673e-04,
                               -1.20689940e-04, -1.16870528e-04, -1.12511443e-04, -1.07517577e-04,
                               -1.01790937e-04, -9.52308758e-05, -8.77343442e-05, -7.91961458e-05,
                               -6.95092062e-05, -5.85648505e-05, -4.62530896e-05, -3.24629142e-05,
                               -1.70825969e-05, 7.00546530e-20, 1.88971100e-05, 3.97207435e-05,
                               6.25823645e-05, 8.75925637e-05, 1.14860730e-04, 1.44494717e-04,
                               1.76600515e-04, 2.11281917e-04, 2.48640189e-04, 2.88773737e-04,
                               3.31777788e-04, 3.77744062e-04, 4.26760455e-04, 4.78910727e-04,
                               5.34274194e-04, 5.92925431e-04, 6.54933979e-04, 7.20364061e-04,
                               7.89274312e-04, 8.61717514e-04, 9.37740346e-04, 1.01738314e-03,
                               1.10067967e-03, 1.18765690e-03, 1.27833484e-03, 1.37272632e-03,
                               1.47083682e-03, 1.57266435e-03, 1.67819927e-03, 1.78742421e-03,
                               1.90031393e-03, 2.01683526e-03, 2.13694702e-03, 2.26059997e-03,
                               2.38773680e-03, 2.51829208e-03, 2.65219230e-03, 2.78935589e-03,
                               2.92969326e-03, 3.07310685e-03, 3.21949126e-03, 3.36873329e-03,
                               3.52071214e-03, 3.67529947e-03, 3.83235962e-03, 3.99174977e-03,
                               4.15332013e-03, 4.31691416e-03, 4.48236881e-03, 4.64951477e-03,
                               4.81817671e-03, 4.98817360e-03, 5.15931899e-03, 5.33142131e-03,
                               5.50428421e-03, 5.67770693e-03, 5.85148457e-03, 6.02540854e-03,
                               6.19926691e-03, 6.37284475e-03, 6.54592461e-03, 6.71828685e-03,
                               6.88971008e-03, 7.05997160e-03, 7.22884778e-03, 7.39611452e-03,
                               7.56154768e-03, 7.72492352e-03, 7.88601912e-03, 8.04461281e-03,
                               8.20048466e-03, 8.35341687e-03, 8.50319421e-03, 8.64960448e-03,
                               8.79243889e-03, 8.93149254e-03, 9.06656478e-03, 9.19745965e-03,
                               9.32398630e-03, 9.44595931e-03, 9.56319914e-03, 9.67553249e-03,
                               9.78279262e-03, 9.88481972e-03, 9.98146124e-03, 1.00725722e-02,
                               1.01580156e-02, 1.02376623e-02, 1.03113920e-02, 1.03790929e-02,
                               1.04406620e-02, 1.04960055e-02, 1.05450392e-02, 1.05876879e-02,
                               1.06238866e-02, 1.06535797e-02, 1.06767217e-02, 1.06932772e-02,
                               1.07032206e-02, 1.07065368e-02, 1.07032206e-02, 1.06932772e-02,
                               1.06767217e-02, 1.06535797e-02, 1.06238866e-02, 1.05876879e-02,
                               1.05450392e-02, 1.04960055e-02, 1.04406620e-02, 1.03790929e-02,
                               1.03113920e-02, 1.02376623e-02, 1.01580156e-02, 1.00725722e-02,
                               9.98146124e-03, 9.88481972e-03, 9.78279262e-03, 9.67553249e-03,
                               9.56319914e-03, 9.44595931e-03, 9.32398630e-03, 9.19745965e-03,
                               9.06656478e-03, 8.93149254e-03, 8.79243889e-03, 8.64960448e-03,
                               8.50319421e-03, 8.35341687e-03, 8.20048466e-03, 8.04461281e-03,
                               7.88601912e-03, 7.72492352e-03, 7.56154768e-03, 7.39611452e-03,
                               7.22884778e-03, 7.05997160e-03, 6.88971008e-03, 6.71828685e-03,
                               6.54592461e-03, 6.37284475e-03, 6.19926691e-03, 6.02540854e-03,
                               5.85148457e-03, 5.67770693e-03, 5.50428421e-03, 5.33142131e-03,
                               5.15931899e-03, 4.98817360e-03, 4.81817671e-03, 4.64951477e-03,
                               4.48236881e-03, 4.31691416e-03, 4.15332013e-03, 3.99174977e-03,
                               3.83235962e-03, 3.67529947e-03, 3.52071214e-03, 3.36873329e-03,
                               3.21949126e-03, 3.07310685e-03, 2.92969326e-03, 2.78935589e-03,
                               2.65219230e-03, 2.51829208e-03, 2.38773680e-03, 2.26059997e-03,
                               2.13694702e-03, 2.01683526e-03, 1.90031393e-03, 1.78742421e-03,
                               1.67819927e-03, 1.57266435e-03, 1.47083682e-03, 1.37272632e-03,
                               1.27833484e-03, 1.18765690e-03, 1.10067967e-03, 1.01738314e-03,
                               9.37740346e-04, 8.61717514e-04, 7.89274312e-04, 7.20364061e-04,
                               6.54933979e-04, 5.92925431e-04, 5.34274194e-04, 4.78910727e-04,
                               4.26760455e-04, 3.77744062e-04, 3.31777788e-04, 2.88773737e-04,
                               2.48640189e-04, 2.11281917e-04, 1.76600515e-04, 1.44494717e-04,
                               1.14860730e-04, 8.75925637e-05, 6.25823645e-05, 3.97207435e-05,
                               1.88971100e-05, 7.00546530e-20, -1.70825969e-05, -3.24629142e-05,
                               -4.62530896e-05, -5.85648505e-05, -6.95092062e-05, -7.91961458e-05,
                               -8.77343442e-05, -9.52308758e-05, -1.01790937e-04, -1.07517577e-04,
                               -1.12511443e-04, -1.16870528e-04, -1.20689940e-04, -1.24061673e-04,
                               -1.27074397e-04, -1.29813259e-04, -1.32359696e-04, -1.34791263e-04,
                               -1.37181472e-04, -1.39599651e-04, -1.42110809e-04, -1.44775525e-04,
                               -1.47649846e-04, -1.50785197e-04, -1.54228316e-04};
typedef struct
{
    QueueHandle_t report_queue;
    calibration_t *calibration;
    blank_take_t *blank_take;
    float *sample_take;
    float *absorbance;
    uint8_t * percentage;
} taskParam_t;

void SpecSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_CHECK;
    check_result = (uint16_t *)malloc(sizeof(uint16_t));
    *check_result = 0;
    percentage = (uint8_t*)malloc(sizeof(uint8_t));
    *percentage = 0;
    sctp_lcd_spec_sample_waiting(cursor, *check_result);
}

void filterAbsorbance(float * input, float * output, int length) {
    ESP_LOGI(TAG, "zero init");
    for (int i=0; i< length; i++) {
        output[i] = 0;
    }

    ESP_LOGI(TAG, "start");
    for (int i=0; i < length; i++) {
        for (int j=0; j<251; j++) {
            int pseu_i = i - 125 + j;
            if ((pseu_i >= 0) && (pseu_i<length)) {
                output[i] += filter[j] * input[pseu_i];
            }
        }
    }
    ESP_LOGI(TAG, "filter end");

}

static void takeSpectrumSample(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    float * sample_take = ((taskParam_t *) pvParameters)->sample_take;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;
    uint8_t * percentage = ((taskParam_t *) pvParameters)->percentage;

	esp_err_t report = sctp_sensor_spectrum_sample(calibration, blank_take, sample_take, percentage);

    float * absorbance = ((taskParam_t *) pvParameters)->absorbance;

	// castings
	float * blank_buffer = blank_take->readout;
	float * sample_buffer = sample_take;
	for (int i=0; i < calibration->length; i++) {
		float transmission = sample_buffer[i]/blank_buffer[i];
		absorbance[i] = -log10(transmission);
	}

    float * unfiltered = (float *) malloc(sizeof(float) * calibration->length);
    memcpy(unfiltered, absorbance, sizeof(float) * calibration->length);
	filterAbsorbance(unfiltered, absorbance, calibration->length);
    free(unfiltered);

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
                    sctp_lcd_spec_sample_clear(cursor);
                    sctp_lcd_spec_sample_waiting(cursor, *check_result);
                    cursor = CURSOR_NULL;
                    sctp_lcd_spec_sample_sampling(cursor, *percentage);

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
                	((taskParam_t *) taskParam)->percentage = percentage;

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
                    free(sctp->blank_take);
                    sctp->blank_take = NULL;
                    sctp->setState(SpecBlank::getInstance());
                    break;
                }
            }
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_CANCEL: {
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
                    sctp_lcd_spec_sample_sampling(cursor, *percentage);
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
        } else {
           sctp_lcd_spec_blank_sampling_percentage(*percentage);
        }
    }
}

void SpecSample::exit(Sctp *sctp)
{
    free(check_result);
    free(percentage);
}

SctpState& SpecSample::getInstance()
{
	static SpecSample singleton;
	return singleton;
}