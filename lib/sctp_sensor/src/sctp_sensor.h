#pragma once

#include <esp_err.h>

#include "sctp_common_types.h"

esp_err_t sctp_sensor_spectrum_blank(calibration_t calibration, blank_sample_t blank_sample);
