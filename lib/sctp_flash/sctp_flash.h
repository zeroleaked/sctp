#pragma once

#include <esp_err.h>
#include <stdint.h>

#include "sctp_common_types.h"

esp_err_t sctp_flash_save_spectrum(float * absorbance, float * wavelength, uint16_t length);

esp_err_t sctp_flash_load_curve_list(curve_t curves[6]);
