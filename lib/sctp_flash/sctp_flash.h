#pragma once

#include <esp_err.h>
#include <stdint.h>

esp_err_t sctp_flash_save_spectrum(float * absorbance, uint16_t spectrum_length);