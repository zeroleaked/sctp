/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <unity.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

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

static const char *TAG = "example";

#define STORAGE_NAMESPACE "storage"
#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  GPIO_NUM_4
#define PIN_NUM_MOSI  GPIO_NUM_5
#define PIN_NUM_CLK   GPIO_NUM_6
#define PIN_NUM_CS    GPIO_NUM_7

#define FILE_LEN 60
#define NAME_LEN 25

typedef struct {
	double gain;
	double bias;

	uint16_t row;
	uint16_t start; // pixel column index
	uint16_t length; // length of full spectrum
} calibration_t;

typedef struct {
	char * filename; // curve1_400nm.csv\0 (17 char) 
	// char filename[20]; // curve1_400nm.csv\0 (17 char) 
	uint16_t wavelength;
	uint8_t id;
	uint8_t points;
	float * absorbance;
	float * concentration;
} curve_t;

typedef struct
{
    // char *filename; // curve1_400nm.csv\0 (17 char)
    char filename[20];    // curve1_400nm.csv\0 (17 char) // 20
    uint16_t wavelength;  // 2
    uint8_t id;           // 1
    uint8_t points;       // 1
    float absorbance[10]; // 40
    float concentration[10];
} flash_curve_t;

typedef struct
{
    uint8_t id;
    uint8_t measurement_mode;
    char *filename;
} history_t; // 20 bytes

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

esp_err_t sctp_flash_save_calibration(calibration_t data, char * filename) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);
    // First create a file.
    char file_cal[] = "/sdcard/calib.csv";
    strcpy(filename, "calib.csv");

    ESP_LOGI(TAG, "Opening file %s", file_cal);
    FILE *f = fopen(file_cal, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "gain, %.10f\n", data.gain);
    fprintf(f, "bias, %.10f\n", data.bias);
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
    char file_cal[NAME_LEN];
    strcpy(file_cal, "/sdcard/");
    strcat(file_cal, filename);
    char *temp;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);
    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_cal);
    FILE *f = fopen(file_cal, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
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

esp_err_t sctp_flash_load_history_list(history_t list[FILE_LEN])
{
    uint8_t count = 0;
    struct stat sb;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t *card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);
    char spec_dir[] = "/sdcard/spectrum";
    struct dirent *de;
    DIR *d = opendir(spec_dir);
    if (d == NULL)
    {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    char *temp;
    while ((de = readdir(d)) != NULL)
    {
        temp = de->d_name;
        if (temp[0] != '_' && temp[0] != '.')
        {
            strcpy(list[count].filename, temp);
            list[count].id = count + 1;
            list[count].measurement_mode = 1;
            ESP_LOGI(TAG, "%d, %s", list[count].id, temp);
            count++;
        }
    }
    char curve_dir[] = "/sdcard/curves";
    // ESP_LOGI(TAG, "curve directory: %s", curve_dir);
    d = opendir(curve_dir);
    if (d == NULL)
    {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    while ((de = readdir(d)) != NULL)
    {
        temp = de->d_name;
        if (temp[0] != '_' && temp[0] != '.')
        {
            strcpy(list[count].filename, temp);
            list[count].id = count + 1;
            list[count].measurement_mode = 1;
            ESP_LOGI(TAG, "%d, %s", list[count].id, temp);
            count++;
        }
    }
    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_load_curve_list(curve_t curves[6]) {
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t * card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);

    char *temp;
    struct dirent *de;
    // char dir_name[] = ".";
    int i = 0;
    DIR *d = opendir(".");
    if (d == NULL) {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    while ((de = readdir(d)) != NULL) {
        temp = de->d_name;
        if(temp[0] != '_' && temp[0] != '.') {
            // strcpy(curves[i].filename, temp);
            ESP_LOGI(TAG, "%s", temp);
            i++;
        }
    }
    closedir(d);
    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

void sctp_flash_nvs_init() {
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

esp_err_t sctp_flash_nvs_save_calibration(calibration_t calibration) {
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Opening failed.");
        return err;
    }

    // Initialize the size of memory space required for blob
    calibration_t *flash_calibration;
    size_t required_size = sizeof(calibration_t);
    flash_calibration = malloc(required_size);

    flash_calibration->gain = calibration.gain;
    flash_calibration->bias = calibration.bias;
    flash_calibration->row = calibration.row;
    flash_calibration->start = calibration.start;
    flash_calibration->length = calibration.length;

    // Write value
    err = nvs_set_blob(my_handle, "calibration", flash_calibration, required_size);
    free(flash_calibration);

    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Writing failed.");
        return err;
    }

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Saving failed.");
        return err;
    } else {
        ESP_LOGI(TAG, "Successfully saved calibration values to nvs.");
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
    }

esp_err_t sctp_flash_nvs_load_calibration(calibration_t *calibration) {
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Read run time blob
    size_t required_size = sizeof(calibration_t); // value will default to 0, if not set yet in NVS
    // obtain required memory space to store blob being read from NVS
    err = nvs_get_blob(my_handle, "calibration", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        return err;
    if (required_size == 0)
    {
        ESP_LOGI(TAG, "Nothing saved yet!\n");
    }
    else
    {
        err = nvs_get_blob(my_handle, "calibration", calibration, &required_size);
        if (err != ESP_OK)
        {
            free(calibration);
            return err;
        } else {
            ESP_LOGI(TAG, "Successfully loaded calibration values from nvs.");
        }

        ESP_LOGI(TAG, "gain: %.10f", (double)calibration->gain);
        ESP_LOGI(TAG, "bias: %.10f", (double)calibration->bias);
        ESP_LOGI(TAG, "row: %d", calibration->row);
        ESP_LOGI(TAG, "start: %d", calibration->start);
        ESP_LOGI(TAG, "length: %d", calibration->length);

        free(calibration);
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_nvs_save_curve(curve_t curves[6]) {
    nvs_handle_t my_handle;
    esp_err_t err;
    char key[20];

    sctp_flash_nvs_init();
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Initialize the size of memory space required for blob
    flash_curve_t *flash_curve;
    size_t required_size = sizeof(flash_curve_t);
    flash_curve = malloc(required_size);
    for(int i=0; i<6; i++) {
        // ESP_LOGI(TAG, "i=%d", i);
        if(curves[i].wavelength==0)
            continue;
        flash_curve->id = curves[i].id;
        flash_curve->points = curves[i].points;
        flash_curve->wavelength = curves[i].wavelength;
        // ESP_LOGI(TAG, "CHECKPOINT 1");
        strcpy(flash_curve->filename, curves[i].filename);
        // ESP_LOGI(TAG, "CHECKPOINT 2");
        for (int j = 0; j < curves[i].points; j++)
        {
            flash_curve->concentration[j] = (curves[i].concentration)[j];
            flash_curve->absorbance[j] = (curves[i].absorbance)[j];
        }
        // ESP_LOGI(TAG, "CHECKPOINT 3");
        // Write value
        sprintf(key, "curve_%d", i+1);
        err = nvs_set_blob(my_handle, key, flash_curve, required_size);
    }
    free(flash_curve);

    if (err != ESP_OK)
        return err;

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Save failed.");
        return err;
    } else
        ESP_LOGI(TAG, "Successfully saved curve values to nvs.");

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_nvs_load_curve(curve_t curves[6]) {
    nvs_handle_t my_handle;
    esp_err_t err;

    sctp_flash_nvs_init();
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Read run time blob
    size_t required_size = sizeof(flash_curve_t); // value will default to 0, if not set yet in NVS
    // obtain required memory space to store blob being read from NVS
    err = nvs_get_blob(my_handle, "flash_curve", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        return err;
    if (required_size == 0)
    {
        ESP_LOGI(TAG, "Nothing saved yet!\n");
    }
    else
    {
        flash_curve_t *flash_curve = malloc(required_size);
        char key[20];
        for(int i=0; i<6; i++) {
            if(curves[i].wavelength==0)
                continue;
            sprintf(key, "curve_%d", i+1);
            err = nvs_get_blob(my_handle, key, flash_curve, &required_size);
            curves[i].id = flash_curve->id;
            curves[i].points = flash_curve->points;
            curves[i].wavelength = flash_curve->wavelength;
            strcpy(curves[i].filename, flash_curve->filename);
            for (int j = 0; j < curves[i].points; j++)
            {
                (curves[i].concentration)[j] = flash_curve->concentration[j];
                (curves[i].absorbance)[j] = flash_curve->absorbance[j];
            }
            if (err != ESP_OK)
            {
                free(flash_curve);
                return err;
            } else
                ESP_LOGI(TAG, "Successfully loaded slot no. %d from nvs.", i+1);
            ESP_LOGI(TAG, "id: %d", curves[i].id);
            ESP_LOGI(TAG, "wavelength: %d", curves[i].wavelength);
            ESP_LOGI(TAG, "points: %d", curves[i].points);
            ESP_LOGI(TAG, "filename: %s", curves[i].filename);
            for(int j=0; j < curves[i].points; j++) {
                ESP_LOGI(TAG, "%.3f, %.3f", (double)(curves[i].concentration)[j], (double)(curves[i].absorbance)[j]);
            }
        }
        free(flash_curve);
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

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

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test1);
    UNITY_END();
}