#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "sctp_flash.h"
#include "sctp_common_types.h"

static uint8_t next_id = 0;

esp_err_t sctp_flash_save_spectrum(float * absorbance, uint16_t spectrum_length) {
    assert(next_id < 60);
    vTaskDelay(2000 / portTICK_RATE_MS);

    next_id++;

    return ESP_OK;
};
