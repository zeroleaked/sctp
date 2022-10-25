#include <unity.h>
#include <esp_log.h>

#include "sctp_flash.h"

const static char TAG[] = "test_flash";

    void test1(void)
    {
        // Use POSIX and C standard library functions to work with files.

        // sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        // sdmmc_card_t * card;

        // sctp_flash_init(PIN_NUM_CS, &host, &card);

        // curve_t curves[6];
        // curve_t loaded[6];
        // ESP_LOGI(TAG, "Initialized some variables as follows:");
        // for(int i=0; i<6; i++) {
        //     curves[i].id = i+1;
        //     if(i < 2) {
        //         curves[i].points = 10;
        //         curves[i].wavelength = 500 + i*150;
        //         char filename[NAME_LEN];
        //         sprintf(filename, "%d_%dnm.csv", curves[i].id, curves[i].wavelength);
        //         curves[i].filename = malloc(NAME_LEN);
        //         strcpy(curves[i].filename, filename);
        //         curves[i].concentration = malloc(sizeof(float) * curves[i].points);
        //         curves[i].absorbance = malloc(sizeof(float) * curves[i].points);
        //         for(int j=0; j<curves[i].points; j++) {
        //             (curves[i].concentration)[j] = 0.01 * 10 * (j+1);
        //             (curves[i].absorbance)[j] = 0.05 * 15 * (j+1);
        //         }
        //         ESP_LOGI(TAG, "id: %d", curves[i].id);
        //         ESP_LOGI(TAG, "wavelength: %d", curves[i].wavelength);
        //         ESP_LOGI(TAG, "points: %d", curves[i].points);
        //         ESP_LOGI(TAG, "filename: %s", curves[i].filename);
        //         for (int j = 0; j < curves[i].points; j++)
        //         {
        //             ESP_LOGI(TAG, "%.3f, %.3f", (double)(curves[i].concentration)[j], (double)(curves[i].absorbance)[j]);
        //         }
        //     }
        //     else {
        //         curves[i].wavelength = 0;
        //         ESP_LOGI(TAG, "Slot no. %d is left blank", i+1);
        //     }
        // }


        // for(int i=0; i<6; i++) {
        //     loaded[i].filename = malloc(NAME_LEN);
        //     loaded[i].concentration = malloc(sizeof(float) * 10);
        //     loaded[i].absorbance = malloc(sizeof(float) * 10);
        // }

        // calibration_t calibration;
        // calibration_t calibration_load;
        // double gain = 0;
        // double bias = 0;
        // uint16_t row = 0;
        // uint16_t start = 0;  // pixel column index
        // uint16_t length = 0; // length of full spectrum
        // calibration_load.gain = gain;
        // calibration_load.bias = bias;
        // calibration_load.row = row;
        // calibration_load.start = start;
        // calibration_load.length = length;
        // char *calibration_file;
        // calibration_file = malloc(sizeof(char) * NAME_LEN);
        // calibration.row = 496;
        // calibration.gain = -0.7698064209;
        // calibration.bias = 1025.924915;
        // calibration.start = 423;
        // calibration.length = 392;

        // ESP_LOGI(TAG, "%.10f", calibration.gain);
        // ESP_LOGI(TAG, "%.10f", calibration.bias);
        // ESP_LOGI(TAG, "%d", calibration.row);
        // ESP_LOGI(TAG, "%d", calibration.start);
        // ESP_LOGI(TAG, "%d", calibration.length);

        // sctp_flash_nvs_init();
        // ESP_LOGI(TAG, "Initialized nvs handle.");
        // sctp_flash_nvs_save_curve(curves);
        // sctp_flash_nvs_load_curve(loaded);
        // sctp_flash_nvs_save_calibration(calibration);
        // sctp_flash_nvs_load_calibration(&calibration_load);

        history_t loaded[FILE_LEN];
        for(int i=0; i<FILE_LEN; i++) {
            loaded[i].filename = malloc(NAME_LEN);
        }
        sctp_flash_load_history_list(loaded);

        // curve_t loaded[6];
        // sctp_flash_load_curve_list(loaded);
        // for(int i=0;i<6;i++) {
        //     ESP_LOGI(TAG, "%s", loaded[i].filename);
        // }

        // gpio_pullup_en(PIN_NUM_CS);
        // gpio_pullup_en(PIN_NUM_MISO);
        // gpio_pullup_en(PIN_NUM_CLK);

        // sctp_flash_save_calibration(calibration, calibration_file);
        // sctp_flash_load_calibration(&calibration_load, calibration_file);
    }

void nvs_curve_save() {
    curve_t curve;
    curve.absorbance = malloc(sizeof(float) * 10);
    curve.concentration = malloc(sizeof(float) * 10);
    ESP_LOGI(TAG, "malloc ok");
    curve.id = 0;
    curve.points = 10;
    curve.wavelength = 554;

    ESP_LOGI(TAG, "loading floats");
    for (int i=0; i< 10; i++) {
        curve.absorbance[i] = (i+1);
        curve.concentration[i] = (i+1)*0.1;
    }
    ESP_LOGI(TAG, "loading floats");

    ESP_LOGI(TAG, "saving");
    sctp_flash_nvs_save_curve(&curve);

    free(curve.absorbance);
    free(curve.concentration);
}

void app_main(void)
{
    UNITY_BEGIN();
    // RUN_TEST(test1);

    RUN_TEST(nvs_curve_save);
    UNITY_END();
}