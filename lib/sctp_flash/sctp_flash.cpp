#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_log.h>

#include "sctp_flash.h"

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

static const char TAG[] = "sctp_flash";
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static uint8_t next_id = 0;

void sctp_flash_init(gpio_num_t cs_gpio, sdmmc_host_t * host, sdmmc_card_t ** card) {
    esp_err_t ret;
    ESP_LOGI(TAG, "HELLO WORLD :)");
    gpio_pullup_en(PIN_NUM_MOSI);

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host->slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = cs_gpio;
    slot_config.host_id = host->slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    const char mount_point[] = MOUNT_POINT;
    ret = esp_vfs_fat_sdspi_mount(mount_point, host, &slot_config, &mount_config, card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, *card);
}

void sctp_flash_deinit(sdmmc_host_t * host, sdmmc_card_t * card) {
    //sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    // All done, unmount partition and disable SPI peripheral
    const char mount_point[] = MOUNT_POINT;
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host->slot);
}

esp_err_t sctp_flash_save_spectrum(float * absorbance, float * wavelength, uint16_t length) {
    uint8_t check;
    uint8_t count = 0;
    char* dir_name = "/sdcard/spectrum";
    struct stat sb;
    struct dirent *de;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_EXT, &host, &card);
    if(stat(dir_name, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    } else {
        check = mkdir(dir_name, 0777);
    }
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    char * temp;
    while ((de = readdir(dir)) != NULL) {
        temp = de->d_name;
        if(temp[0] != '_' && temp[0] != '.')
            count++;
    }
    closedir(dir);

    // Create a file.
    char file_spec[] = "/sdcard/spectrum/spec_XXXX.csv";
    sprintf(file_spec, "/sdcard/spectrum/spec_%d.csv", count+1);

    ESP_LOGI(TAG, "Opening file %s", file_spec);
    FILE *f = fopen(file_spec, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "wavelength, absorbance\n");
    for(int i=0; i < length; i++) {
        fprintf(f, "%.3f, %.3f\n", wavelength[i], absorbance[i]);
    }
    fclose(f);
    ESP_LOGI(TAG, "File written");

    sctp_flash_deinit(&host, card);
    return ESP_OK;
};

esp_err_t sctp_flash_save_curve_int(curve_t curve) {
    int check;
    char* dir_name = "/sdcard/curves";
    struct stat sb;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_INT, &host, &card);
    if(stat(dir_name, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    } else {
        check = mkdir(dir_name, 0777);
    }

    // First create a file.
    char file_curve[] = "/sdcard/curves/XXXX_XXXXnm.csv";
    sprintf(file_curve, "/sdcard/curves/%d_%dnm.csv", curve.id, curve.wavelength);

    ESP_LOGI(TAG, "Opening file %s", file_curve);
    FILE *f = fopen(file_curve, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "id, %d\n", curve.id);
    fprintf(f, "wavelength, %d\n", curve.wavelength);
    fprintf(f, "index, concentration, absorbance\n");
    for(int i=0; i < curve.points; i++) {
        fprintf(f, "%d, %.3f, %.3f\n", i+1, curve.concentration[i], curve.absorbance[i]);
    }
    fclose(f);
    ESP_LOGI(TAG, "File written");

    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_save_calibration(calibration_t data, char * filename) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_INT, &host, &card);
    // First create a file.
    char file_cal[] = "/sdcard/calibration.csv";

    ESP_LOGI(TAG, "Opening file %s", file_cal);
    FILE *f = fopen(file_cal, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "gain, %.3f\n", (double)data.gain);
    fprintf(f, "bias, %.3f\n", (double)data.bias);
    fprintf(f, "row, %d\n", data.row);
    fprintf(f, "start, %d\n", data.start);
    fprintf(f, "length, %d\n", data.length);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_load_calibration(calibration_t * data, char * filename) {
    char line[NAME_LEN];
    char file_cal[] = "/sdcard/calibration.csv";
    char *temp;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_INT, &host, &card);
    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_cal);
    FILE *f = fopen(file_cal, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    int i = 0;
    while(fgets(line, sizeof(line), f)) {
        switch(i) {
            case 0: {
                temp = strtok(line, ", ");
                data->gain = (float) atof(strtok(NULL, ", "));
                break;
            }
            case 1: {
                temp = strtok(line, ", ");
                data->bias = (float) atof(strtok(NULL, ", "));
                break;
            }
            case 2: {
                temp = strtok(line, ", ");
                data->row = atoi(strtok(NULL, ", "));
                break;
            }
            case 3: {
                temp = strtok(line, ", ");
                data->start = atoi(strtok(NULL, ", "));
                break;
            }
            case 4: {
                temp = strtok(line, ", ");
                data->length = atoi(strtok(NULL, ", "));
                break;
            }
        }
        i++;
    }
    fclose(f);

    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

const char mockup_curves_filename[][20] = {
    "curve1_400nm.csv",
    "curve2_450nm.csv",
    "curve3_500nm.csv",
    "curve3_500nm.csv",
    "curve4_700nm.csv",
    "NaN"
};

esp_err_t sctp_flash_load_curve_list(curve_t curves[6]) {
    char *temp;
    struct dirent *de;
    char dir_name[] = "/sdcard/curves/";
    int i = 0;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_INT, &host, &card);
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    while ((de = readdir(dir)) != NULL) {
        temp = de->d_name;
        if(temp[0] != '_' && temp[0] != '.') {
            strcpy(curves[i].filename, temp);
            ESP_LOGI(TAG, "%s", curves[i].filename);
            i++;
        }
    }
    closedir(dir);

    sctp_flash_deinit(&host, card);
    return ESP_OK;
    // wavelength = 0 then empty

    // for (int i=0; i<6; i++) {
    //     curves[i].id = i;

    //     if (i < 3) {
    //         curves[i].wavelength = 400 + i*50;
    //         curves[i].points = 5;
    //     }
    //     else if (i == 3) {
    //         curves[i].wavelength = 700;
    //         curves[i].points = 5;
    //     }
    //     else {
    //         curves[i].wavelength = 0;
    //         curves[i].points = 0;
    //     }

    //     strcpy(curves[i].filename, mockup_curves_filename[i]);
    // }
}

esp_err_t sctp_flash_load_curve_floats(curve_t * curve) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_INT, &host, &card);
    
    char line[2 * NAME_LEN];
    char file_curves[] = "/sdcard/curves/";
    char *temp;
    strcat(file_curves, curve->filename);

    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_curves);
    FILE *f = fopen(file_curves, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    int i = 0;
    while(fgets(line, sizeof(line), f)) {
        if(i==0) {
            temp = strtok(line, ", ");
            curve->id = atoi(strtok(NULL, ", "));
        } else if(i==1) {
            temp = strtok(line, ", ");
            curve->wavelength = atoi(strtok(NULL, ", "));
        } else if(i==2) {
        } else {
            temp = strtok(line, ", ");
            int idx = atoi(temp);     
            curve->concentration[idx-1] = (float) atof(strtok(NULL, ", "));
            curve->absorbance[idx-1] = (float) atof(strtok(NULL, ", "));
        }
        i++;
    }
    fclose(f);
    curve->points = i-3;

    // for (int i=0; i < 15; i++) {
    //     curve->absorbance[i] = 0;
    //     curve->concentration[i] = 0;
    //     if (i < curve->points) {
    //         curve->absorbance[i] = (i+1) * 0.1;
    //         curve->concentration[i] = (i+1) * 0.001;
    //     }
    // }
    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_load_history_list(char filenames[FILE_LEN][NAME_LEN]) {
    uint8_t count = 0;
    struct stat sb;
    char temp[NAME_LEN];

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_EXT, &host, &card);
    for(int i=0; i<60; i++) {
        char dir_name[] = "/sdcard/spectrum/";
        sprintf(temp, "spec_%d.csv", i+1);
        strcat(dir_name, temp);
        if(stat(dir_name, &sb) == 0) {
            ESP_LOGI(TAG, "match");
            ESP_LOGI(TAG, "%d, %s", i, temp);
            strcpy(filenames[count], temp);
            count++;
        }
    }
    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_load_spectrum(char * filename, float * absorbance, float * wavelength, uint16_t length) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS_EXT, &host, &card);
    char line[NAME_LEN];
    char file_spec[] = "/sdcard/spectrum/";
    char filename[] = "spec_6.csv";
    strcat(file_spec, filename);

    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_spec);
    FILE *f = fopen(file_spec, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    int i = 0;
    while(fgets(line, sizeof(line), f)) {
        if(i != 0) {
            wavelength[i-1] = (float) atof(strtok(line, ", "));
            absorbance[i-1] = (float) atof(strtok(NULL, ", "));
            ESP_LOGI(TAG, "%d, %.3f, %.3f", i, (double)wavelength[i-1], (double)absorbance[i-1]);
        }
        i++;
    }
    fclose(f);
    length = i-1;
    sctp_flash_deinit(&host, card);
    return ESP_OK;
}