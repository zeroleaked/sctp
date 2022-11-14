#include <unity.h>
#include <esp_log.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "sctp_flash.h"

static const char *TAG = "example";

#define STORAGE_NAMESPACE "storage"
#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  GPIO_NUM_5
#define PIN_NUM_MOSI  GPIO_NUM_4
#define PIN_NUM_CLK   GPIO_NUM_6
#define PIN_NUM_CS    GPIO_NUM_7

#define FILE_LEN 60
#define NAME_LEN 25

    void test1(void)
    {
        calibration_t calibration;
        calibration_t calibration_load;
        double gain = 0;
        double bias = 0;
        uint16_t row = 0;
        uint16_t start = 0;  // pixel column index
        uint16_t length = 0; // length of full spectrum
        calibration_load.gain = gain;
        calibration_load.bias = bias;
        calibration_load.row = row;
        calibration_load.start = start;
        calibration_load.length = length;
        // char *calibration_file;
        // calibration_file = malloc(sizeof(char) * NAME_LEN);
        calibration.row = 490;
        calibration.gain = -0.7666855524;
        calibration.bias = 1013.975014;
        calibration.start = 409;
        calibration.length = 393;

        // sctp_flash_nvs_init();
        // ESP_LOGI(TAG, "Initialized nvs handle.");
        // sctp_flash_nvs_save_curve(curves);
        // sctp_flash_nvs_load_curve(loaded);
        sctp_flash_nvs_save_calibration(calibration);
        ESP_LOGI(TAG, "%.10f", calibration.gain);
        ESP_LOGI(TAG, "%.10f", calibration.bias);
        ESP_LOGI(TAG, "%d", calibration.row);
        ESP_LOGI(TAG, "%d", calibration.start);
        ESP_LOGI(TAG, "%d", calibration.length);
        // sctp_flash_nvs_load_calibration(&calibration_load);

        // history_t loaded[FILE_LEN];
        // for(int i=0; i<FILE_LEN; i++) {
        //     loaded[i].filename = malloc(NAME_LEN);
        // }
        // sctp_flash_load_history_list(loaded);

        // curve_t loaded[6];
        // sctp_flash_load_curve_list(loaded);
        // for(int i=0;i<6;i++) {
        //     ESP_LOGI(TAG, "%s", loaded[i].filename);
        // }

        // gpio_pullup_en(PIN_NUM_CS);
        // gpio_pullup_en(PIN_NUM_MISO);
        // gpio_pullup_en(PIN_NUM_CLK);

        // sctp_flash_save_calibration(calibration, calibration_file);
        // sctp_flash_load_calibration(&calibration_load);
        // ESP_LOGI(TAG, "%.10f", calibration_load.gain);
        // ESP_LOGI(TAG, "%.10f", calibration_load.bias);
        // ESP_LOGI(TAG, "%d", calibration_load.row);
        // ESP_LOGI(TAG, "%d", calibration_load.start);
        // ESP_LOGI(TAG, "%d", calibration_load.length);
    }

void nvs_curve_save() {
    curve_t curve;
    curve.absorbance = malloc(sizeof(float) * 10);
    curve.concentration = malloc(sizeof(float) * 10);
    curve.id = 1;
    curve.points = 6;
    curve.wavelength = 554;

    for (int i=0; i< 10; i++) {
        if (i<curve.points) {
            curve.absorbance[i] = (i+1);
            curve.concentration[i] = (i+1)*0.1;
        }
        else {
            curve.absorbance[i] = -1;
            curve.concentration[i] = 0;
        }
    }

    esp_err_t ret = sctp_flash_nvs_save_curve(&curve);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    free(curve.absorbance);
    free(curve.concentration);
}

void nvs_curve_list() {
    curve_t curves[6];

    esp_err_t ret = sctp_flash_nvs_load_curve_list(curves);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    for (int i=0; i<6; i++) {
        if (curves[i].wavelength != 0) {
            ESP_LOGI(TAG, "curve_%d: %d %d %d", i, curves[i].id, curves[i].wavelength, curves[i].points);
        }
        else ESP_LOGI(TAG, "no curve_%d", i);
    }
}

void nvs_curve_load() {
    curve_t curve;
    curve.absorbance = malloc(sizeof(float) * 10);
    curve.concentration = malloc(sizeof(float) * 10);
    curve.id = 5;
    curve.points = 10;
    curve.wavelength = 554;

    esp_err_t ret = sctp_flash_nvs_load_curve(&curve);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    for (int i=0; i<10; i++) {
        ESP_LOGI(TAG, "A[%d]=%f; C[%d]=%f", i, curve.absorbance[i], i, curve.concentration[i]);
    }

    free(curve.absorbance);
    free(curve.concentration);
}

void app_main(void)
{
    UNITY_BEGIN();
    // RUN_TEST(test1);

    RUN_TEST(nvs_curve_save);
    // RUN_TEST(nvs_curve_list);
    // RUN_TEST(nvs_curve_load);

    UNITY_END();
}