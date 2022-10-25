#pragma once

#include <esp_err.h>

#include "sctp_common_types.h"

#define FILE_LEN        60
#define NAME_LEN        25

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sctp_flash_nvs_save_calibration(calibration_t calibration);
esp_err_t sctp_flash_nvs_load_calibration(calibration_t *calibration);

esp_err_t sctp_flash_nvs_save_curve(curve_t * curve);
esp_err_t sctp_flash_nvs_load_curve_l(curve_t curves[6]);
esp_err_t sctp_flash_nvs_load_curve_list(curve_t curves[6]);

void sctp_flash_nvs_erase_all();

esp_err_t sctp_flash_save_spectrum(float *absorbance, float *wavelength, uint16_t length);
esp_err_t sctp_flash_save_curve(curve_t curve);

esp_err_t sctp_flash_load_history_list(history_t list[FILE_LEN]);
esp_err_t sctp_flash_load_spectrum(char * filename, float * absorbance, float * wavelength);
esp_err_t sctp_flash_load_curve_floats(curve_t *curve);

#ifdef __cplusplus
}
#endif