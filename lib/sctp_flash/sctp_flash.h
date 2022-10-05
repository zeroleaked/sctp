#pragma once

#define FILE_LEN        60
#define NAME_LEN        25

esp_err_t sctp_flash_save_spectrum(float * absorbance, float * wavelength, uint16_t length);
esp_err_t sctp_flash_save_curve(curve_t curve);

esp_err_t sctp_flash_save_calibration(calibration_t data, char * filename);
esp_err_t sctp_flash_load_calibration(calibration_t data, char * filename);

esp_err_t sctp_flash_load_curve_list(curve_t curves[6]);
esp_err_t sctp_flash_load_curve_floats(curve_t * curve);

esp_err_t sctp_flash_load_history_list(char filenames[FILE_LEN][NAME_LEN]);
esp_err_t sctp_flash_load_spectrum(char * filename, float * absorbance, float * wavelength, uint16_t length);