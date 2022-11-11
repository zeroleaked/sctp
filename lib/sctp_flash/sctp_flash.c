#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <stdint.h>

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "sctp_flash.h"

#define MOUNT_POINT "/sdcard"
#define STORAGE_NAMESPACE "storage"

#define PIN_NUM_MISO    GPIO_NUM_5
#define PIN_NUM_MOSI    GPIO_NUM_4
#define PIN_NUM_CLK     GPIO_NUM_6
#define PIN_NUM_CS      GPIO_NUM_7

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

void sctp_flash_nvs_init();

static const char TAG[] = "sctp_flash";

void sctp_flash_init(gpio_num_t cs_gpio, sdmmc_host_t * host, sdmmc_card_t ** card)
{
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
        .allocation_unit_size = 16 * 1024};

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
    ret = spi_bus_initialize((spi_host_device_t)host->slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = cs_gpio;
    slot_config.host_id = (spi_host_device_t) (spi_host_device_t)host->slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    const char mount_point[] = MOUNT_POINT;
    ret = esp_vfs_fat_sdspi_mount(mount_point, host, &slot_config, &mount_config, card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, *card);
}

void sctp_flash_deinit(sdmmc_host_t * host, sdmmc_card_t * card)
{
    // sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    //  All done, unmount partition and disable SPI peripheral
    const char mount_point[] = MOUNT_POINT;
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free((spi_host_device_t) (spi_host_device_t)host->slot);
}

void sctp_flash_nvs_init()
{
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

void sctp_flash_nvs_erase_all()
{
    esp_err_t err = nvs_flash_init();
    nvs_handle_t my_handle;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    nvs_erase_all(my_handle);
}

esp_err_t sctp_flash_nvs_save_calibration(calibration_t calibration)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    sctp_flash_nvs_init();
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
    flash_calibration = (calibration_t *) (calibration_t *)malloc(required_size);

    flash_calibration->gain = calibration.gain;
    flash_calibration->bias = calibration.bias;
    flash_calibration->row = calibration.row;
    flash_calibration->start = calibration.start;
    flash_calibration->length = calibration.length;

    // Write value
    err = nvs_set_blob(my_handle, "calibration", flash_calibration, required_size);
    free(flash_calibration);

    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Writing failed.");
        return err;
    }

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Saving failed.");
        return err;
    }
    else
    {
        ESP_LOGI(TAG, "Successfully saved calibration values to nvs.");
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_nvs_load_calibration(calibration_t *calibration)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    sctp_flash_nvs_init();
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
            // free(calibration);
            return err;
        }
        else
        {
            ESP_LOGI(TAG, "Successfully loaded calibration values from nvs.");
        }

        ESP_LOGI(TAG, "gain: %.10f", (double)calibration->gain);
        ESP_LOGI(TAG, "bias: %.10f", (double)calibration->bias);
        ESP_LOGI(TAG, "row: %d", calibration->row);
        ESP_LOGI(TAG, "start: %d", calibration->start);
        ESP_LOGI(TAG, "length: %d", calibration->length);

        // free(calibration);
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_load_calibration(calibration_t *data)
{
    char line[NAME_LEN];
    char file_cal[] = "/sdcard/calib.csv";
    char *temp;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t *card;
    sctp_flash_init(PIN_NUM_CS, &host, &card);

    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_cal);
    FILE *f = fopen(file_cal, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    int i = 0;
    while (fgets(line, sizeof(line), f))
    {
        switch (i)
        {
        case 0:
        {
            temp = strtok(line, ", ");
            data->gain = (float)atof(strtok(NULL, ", "));
            break;
        }
        case 1:
        {
            temp = strtok(line, ", ");
            data->bias = (float)atof(strtok(NULL, ", "));
            break;
        }
        case 2:
        {
            temp = strtok(line, ", ");
            data->row = atoi(strtok(NULL, ", "));
            break;
        }
        case 3:
        {
            temp = strtok(line, ", ");
            data->start = atoi(strtok(NULL, ", "));
            break;
        }
        case 4:
        {
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

esp_err_t sctp_flash_nvs_save_curve(curve_t *curve)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    char key[20];

    sctp_flash_nvs_init();
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Initialize the size of memory space required for blob
    flash_curve_t fc;
    flash_curve_t *flash_curve = &fc;

    flash_curve->id = curve->id;
    flash_curve->points = curve->points;
    flash_curve->wavelength = curve->wavelength;
    memcpy(flash_curve->absorbance, curve->absorbance, 10 * sizeof(float));
    memcpy(flash_curve->concentration, curve->concentration, 10 * sizeof(float));

    sprintf(key, "curve_%d", curve->id);
    err = nvs_set_blob(my_handle, key, flash_curve, sizeof(flash_curve_t));

    if (err != ESP_OK)
        return err;

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Save failed.");
        return err;
    }
    else
        ESP_LOGI(TAG, "Successfully saved curve values to nvs.");

    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_nvs_load_curve(curve_t *curve)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    sctp_flash_nvs_init();
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Read run time blob
    // obtain required memory space to store blob being read from NVS
    char key[20];
    sprintf(key, "curve_%d", curve->id);
    const size_t required_size = sizeof(flash_curve_t); // value will default to 0, if not set yet in NVS

    flash_curve_t fc;
    flash_curve_t *flash_curve = &fc;
    err = nvs_get_blob(my_handle, key, flash_curve, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        return err;
    if (required_size == 0)
    {
        ESP_LOGI(TAG, "Nothing saved yet!\n");
        return err;
    }
    else
    {
        memcpy(curve->absorbance, flash_curve->absorbance, sizeof(float) * 10);
        memcpy(curve->concentration, flash_curve->concentration, sizeof(float) * 10);
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_nvs_load_curve_l(curve_t curves[6])
{
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
        flash_curve_t *flash_curve = (flash_curve_t *) (flash_curve_t *)malloc(required_size);
        char key[20];
        for (int i = 0; i < 6; i++)
        {
            if (curves[i].wavelength == 0)
                continue;
            sprintf(key, "curve_%d", i + 1);
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
            }
            else
                ESP_LOGI(TAG, "Successfully loaded slot no. %d from nvs.", i + 1);
            ESP_LOGI(TAG, "id: %d", curves[i].id);
            ESP_LOGI(TAG, "wavelength: %d", curves[i].wavelength);
            ESP_LOGI(TAG, "points: %d", curves[i].points);
            ESP_LOGI(TAG, "filename: %s", curves[i].filename);
            for (int j = 0; j < curves[i].points; j++)
            {
                ESP_LOGI(TAG, "%.3f, %.3f", (double)(curves[i].concentration)[j], (double)(curves[i].absorbance)[j]);
            }
        }
        free(flash_curve);
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t sctp_flash_nvs_load_curve_list(curve_t curves[6])
{
    nvs_handle_t my_handle;
    esp_err_t err;

    sctp_flash_nvs_init();
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    // Read run time blob
    const size_t required_size = sizeof(flash_curve_t); // value will default to 0, if not set yet in NVS
    // obtain required memory space to store blob being read from NVS
    flash_curve_t fc;
    flash_curve_t *flash_curve = &fc;
    char key[20];
    for (int i = 0; i < 6; i++)
    {
        curves[i].id = i;
        sprintf(key, "curve_%d", i);
        err = nvs_get_blob(my_handle, key, flash_curve, &required_size);
        if (err == ESP_OK)
        {
            curves[i].points = flash_curve->points;
            curves[i].wavelength = flash_curve->wavelength;
        }
        else
        {
            curves[i].wavelength = 0;
        }
    }
    // Close
    nvs_close(my_handle);
    return ESP_OK;
}

const char mockup_curves_filename[][20] = {
    "curve1_400nm.csv",
    "curve2_450nm.csv",
    "curve3_500nm.csv",
    "curve3_500nm.csv",
    "curve4_700nm.csv",
    "NaN"};

esp_err_t sctp_flash_save_spectrum(float *absorbance, float *wavelength, uint16_t length)
{
    uint8_t check;
    uint8_t count = 0;
    char *dir_name = "/sdcard/spectrum";
    struct stat sb;
    struct dirent *de;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t *card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);
    if (stat(dir_name, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
    }
    else
    {
        check = mkdir(dir_name, 0777);
    }
    DIR *dir = opendir(dir_name);
    if (dir == NULL)
    {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    char *temp;
    while ((de = readdir(dir)) != NULL)
    {
        temp = de->d_name;
        if (temp[0] != '_' && temp[0] != '.')
            count++;
    }
    closedir(dir);

    // Create a file.
    char file_spec[] = "/sdcard/spectrum/spec_XXXX.csv";
    sprintf(file_spec, "/sdcard/spectrum/spec_%d.csv", count + 1);

    ESP_LOGI(TAG, "Opening file %s", file_spec);
    FILE *f = fopen(file_spec, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "wavelength, absorbance\n");
    for (int i = 0; i < length; i++)
    {
        fprintf(f, "%.3f, %.3f\n", wavelength[i], absorbance[i]);
    }
    fclose(f);
    ESP_LOGI(TAG, "File written");

    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_save_curve(curve_t curve)
{
    uint8_t check;
    char *dir_name = "/sdcard/curves";
    struct stat sb;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t *card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);
    if (stat(dir_name, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
    }
    else
    {
        check = mkdir(dir_name, 0777);
    }

    // Create a file.
    char file_curve[] = "/sdcard/curves/XX_XXXXXXnm.csv";
    sprintf(file_curve, "/sdcard/curves/%d_%dnm.csv", curve.id, curve.wavelength);
    strcpy(curve.filename, &file_curve[15]);

    ESP_LOGI(TAG, "Opening file %s", file_curve);
    FILE *f = fopen(file_curve, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "id, %d", curve.id);
    fprintf(f, "wavelength, %d", curve.wavelength);
    fprintf(f, "index, concentration, absorbance\n");
    for (int i = 0; i < curve.points; i++)
    {
        fprintf(f, "%d, %.3f, %.3f\n", i + 1, curve.concentration[i], curve.absorbance[i]);
    }
    fclose(f);
    ESP_LOGI(TAG, "File written");

    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_load_history_list(history_t list[FILE_LEN])
{
    uint8_t count = 0;

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
            list[count].measurement_mode = 0;
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

esp_err_t sctp_flash_load_spectrum(char *filename, float *absorbance, float *wavelength, uint16_t *length)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t *card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);
    *length = 0;
    char line[NAME_LEN];
    char file_spec[] = "/sdcard/spectrum/";
    strcat(file_spec, filename);

    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_spec);
    FILE *f = fopen(file_spec, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    int i = 0;
    while (fgets(line, sizeof(line), f))
    {
        if (i != 0)
        {
            wavelength[i - 1] = (float)atof(strtok(line, ", "));
            absorbance[i - 1] = (float)atof(strtok(NULL, ", "));
            ESP_LOGI(TAG, "%d, %.3f, %.3f", i, (double)wavelength[i - 1], (double)absorbance[i - 1]);
        }
        i++;
    }
    *length = i + 1;
    fclose(f);
    sctp_flash_deinit(&host, card);
    return ESP_OK;
}

esp_err_t sctp_flash_load_curve_floats(curve_t *curve)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdmmc_card_t *card;

    sctp_flash_init(PIN_NUM_CS, &host, &card);

    char line[2 * NAME_LEN];
    char file_curves[] = "/sdcard/curves/";
    char *temp;
    strcat(file_curves, curve->filename);

    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_curves);
    FILE *f = fopen(file_curves, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    int i = 0;
    while (fgets(line, sizeof(line), f))
    {
        if (i == 0)
        {
            temp = strtok(line, ", ");
            curve->id = atoi(strtok(NULL, ", "));
        }
        else if (i == 1)
        {
            temp = strtok(line, ", ");
            curve->wavelength = atoi(strtok(NULL, ", "));
        }
        else if (i == 2)
        {
        }
        else
        {
            temp = strtok(line, ", ");
            int idx = atoi(temp);
            curve->concentration[idx - 1] = (float)atof(strtok(NULL, ", "));
            curve->absorbance[idx - 1] = (float)atof(strtok(NULL, ", "));
        }
        i++;
    }
    fclose(f);
    curve->points = i - 3;

    sctp_flash_deinit(&host, card);
    return ESP_OK;
}