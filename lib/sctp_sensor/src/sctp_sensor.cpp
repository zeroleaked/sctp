#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "sctp_sensor.h"

static const char TAG[] = "sctp_sensor";

esp_err_t sctp_sensor_spectrum_blank(calibration_t calibration, blank_take_t blank_take) {
    vTaskDelay(5000 / portTICK_RATE_MS);

    for (int i = 0; i < calibration.length; i++) {
        blank_take.readout[calibration.length-i-1] = (float)(i+calibration.start) * calibration.gain + calibration.bias;
    }
    blank_take.exposure = 400;
    blank_take.gain = 1;
    
    return ESP_OK;
};

esp_err_t sctp_sensor_spectrum_sample(calibration_t calibration, blank_take_t blank_take, sample_take_t sample_take) {
    vTaskDelay(5000 / portTICK_RATE_MS);

    ESP_LOGI(TAG, "gain=%f, bias=%f, start=%d, length=%d", calibration.gain, calibration.bias, calibration.start, calibration.length);

    for (int i = 0; i < calibration.length; i++) {
        sample_take[calibration.length-i-1] = ( (float)(i+calibration.start) * calibration.gain + calibration.bias ) / 10;
    }

    return ESP_OK;
};