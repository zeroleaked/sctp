#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "sctp_sensor.h"

static const char TAG[] = "sctp_sensor";

esp_err_t sctp_sensor_spectrum_blank(calibration_t calibration, blank_sample_t blank_sample) {
    vTaskDelay(5000 / portTICK_RATE_MS);

    for (int i = calibration.start; i < calibration.length; i++) {
        blank_sample.readout[i] = (float)i * calibration.gain + calibration.bias;
    }
    blank_sample.exposure = 400;
    blank_sample.gain = 1;
    
    return ESP_OK;
};

esp_err_t sctp_sensor_spectrum_sample(calibration_t calibration, blank_sample_t blank_sample, sample_sample_t sample_sample) {
    vTaskDelay(5000 / portTICK_RATE_MS);

    for (int i = calibration.start; i < calibration.length; i++) {
        sample_sample[i] = (float)i * calibration.gain + calibration.bias;
    }

    return ESP_OK;
};