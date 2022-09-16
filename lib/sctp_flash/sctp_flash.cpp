#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include "sctp_flash.h"

static uint8_t next_id = 0;

esp_err_t sctp_flash_save_spectrum(float * absorbance, uint16_t spectrum_length) {
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
    "null"
};

esp_err_t sctp_flash_load_curves(curve_t curves[6]) {
    vTaskDelay(2000 / portTICK_RATE_MS);
    // wavelength = 0 then empty

    for (int i=0; i<6; i++) {
        curves[i].id = i;

        if (i < 4)
            curves[i].wavelength = 400 + i*50;
        else if (i == 4)
            curves[i].wavelength = 700;
        else
            curves[i].wavelength = 0;

        strcpy(curves[i].filename, mockup_curves_filename[i]);
    }

    return ESP_OK;
}
