#pragma once

#include <esp_err.h>

#include "sctp_common_types.h"

esp_err_t sctp_sensor_spectrum_blank(calibration_t * calibration, blank_take_t * blank_take);
esp_err_t sctp_sensor_spectrum_sample(calibration_t * calibration, blank_take_t * blank_take, float * sample_take);

esp_err_t sctp_sensor_concentration_blank(calibration_t * calibration, uint16_t wavelength, blank_take_t * blank_take);
esp_err_t sctp_sensor_concentration_sample(calibration_t * calibration, uint16_t wavelength, blank_take_t * blank_take, float * sample_take);
