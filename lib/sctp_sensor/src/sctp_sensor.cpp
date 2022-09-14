#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "sctp_sensor.h"

static const char TAG[] = "sctp_sensor";

esp_err_t sctp_sensor_spectrum_blank(calibration_t calibration, blank_take_t blank_take) {
    vTaskDelay(5000 / portTICK_RATE_MS);

    for (int i = calibration.start; i < calibration.length; i++) {
        blank_take.readout[i] = (float)i * calibration.gain + calibration.bias;
    }
    blank_take.exposure = 400;
    blank_take.gain = 1;
    
    return ESP_OK;
};

esp_err_t sctp_sensor_spectrum_sample(calibration_t calibration, blank_take_t blank_take, sample_take_t sample_take) {
    vTaskDelay(5000 / portTICK_RATE_MS);

    for (int i = calibration.start; i < calibration.length; i++) {
        sample_take[i] = (float)i * calibration.gain + calibration.bias;
    }

    return ESP_OK;
};