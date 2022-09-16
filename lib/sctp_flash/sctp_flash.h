#pragma once

#include <esp_err.h>
#include <stdint.h>

#include "sctp_common_types.h"

esp_err_t sctp_flash_save_spectrum(float * absorbance, uint16_t spectrum_length);

esp_err_t sctp_flash_load_curves(curve_t curves[6]);
