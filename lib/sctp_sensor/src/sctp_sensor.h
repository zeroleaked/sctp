#pragma once

#include <esp_err.h>

#include "sctp_common_types.h"

esp_err_t sctp_sensor_spectrum_blank(calibration_t calibration, blank_take_t blank_take);
esp_err_t sctp_sensor_spectrum_sample(calibration_t calibration, blank_take_t blank_take, sample_take_t sample_take);
