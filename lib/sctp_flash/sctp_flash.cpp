#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_log.h>

#include "sctp_flash.h"

static const char TAG[] = "sctp_flash";

static uint8_t next_id = 0;

esp_err_t sctp_flash_save_spectrum(float * absorbance, float * wavelength, uint16_t length) {
    if (next_id == 60) {
        return ESP_ERR_NO_MEM;
    }
    vTaskDelay(2000 / portTICK_RATE_MS);

    next_id++;

    return ESP_OK;
};

const char mockup_curves_filename[][20] = {
    "curve1_400nm.csv",
    "curve2_450nm.csv",
    "curve3_500nm.csv",
    "curve3_500nm.csv",
    "curve4_700nm.csv",
    "NaN"
};

esp_err_t sctp_flash_load_curve_list(curve_t curves[6]) {
    vTaskDelay(2000 / portTICK_RATE_MS);
    // wavelength = 0 then empty

    for (int i=0; i<6; i++) {
        curves[i].id = i;

        if (i < 3) {
            curves[i].wavelength = 400 + i*50;
            curves[i].points = 5;
        }
        else if (i == 3) {
            curves[i].wavelength = 700;
            curves[i].points = 5;
        }
        else {
            curves[i].wavelength = 0;
            curves[i].points = 0;
        }

        strcpy(curves[i].filename, mockup_curves_filename[i]);
    }

    return ESP_OK;
}

esp_err_t sctp_flash_load_curve_floats(curve_t * curve) {
    ESP_LOGI(TAG, "Writing to float buffers");
    for (int i=0; i < 15; i++) {
        curve->absorbance[i] = 0;
        curve->concentration[i] = 0;
        if (i < curve->points) {
            curve->absorbance[i] = (i+1) * 0.1;
            curve->concentration[i] = (i+1) * 0.001;
        }
    }
    ESP_LOGI(TAG, "Writing to float buffers fin");

    return ESP_OK;
}