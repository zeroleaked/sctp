#pragma once

#include <esp_err.h>
#include <stdint.h>

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "sctp_common_types.h"

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO    GPIO_NUM_4
#define PIN_NUM_MOSI    GPIO_NUM_5
#define PIN_NUM_CLK     GPIO_NUM_6
#define PIN_NUM_CS_EXT  GPIO_NUM_7
#define PIN_NUM_CS_INT  GPIO_NUM_15

void sctp_flash_init(sdmmc_card_t * card, gpio_num_t cs_gpio);
void sctp_flash_deinit(sdmmc_card_t * card);

esp_err_t sctp_flash_save_spectrum(float * absorbance, float * wavelength, uint16_t length);
esp_err_t sctp_flash_save_curve(curve_t curve);

esp_err_t sctp_flash_load_curve_list(curve_t curves[6]);
esp_err_t sctp_flash_load_curve_floats(curve_t * curve);

esp_err_t sctp_flash_load_history_list(char ** filenames);
esp_err_t sctp_flash_load_spectrum(char * filename, float * absorbance, float * wavelength, uint16_t length);