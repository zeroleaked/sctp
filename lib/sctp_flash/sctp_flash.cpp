#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_log.h>

#include "sctp_flash.h"

static const char TAG[] = "sctp_flash";
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static uint8_t next_id = 0;

void sctp_flash_init(sdmmc_card_t * card, gpio_num_t cs_gpio) {
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
    const char mount_point[] = MOUNT_POINT;
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
    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS_INT;
    slot_config.host_id = (spi_host_device_t)host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

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
    sdmmc_card_print_info(stdout, card);
}

void sctp_flash_deinit(sdmmc_card_t * card) {
    const char mount_point[] = MOUNT_POINT;
    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free((spi_host_device_t)host.slot);
}

esp_err_t sctp_flash_save_spectrum(float * absorbance, float * wavelength, uint16_t length) {
    uint8_t check;
    uint8_t count = 0;
    char* dir_name = "/sdcard/spectrum";
    struct stat sb;
    struct dirent *de;

    if(stat(dir_name, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    } else {
        check = mkdir(dir_name, 0777);
    }
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    while ((de = readdir(dir)) != NULL)
        count++;
    closedir(dir);

    // Create a file.
    char file_spec[] = "/sdcard/spectrum/spectrum_XXX.csv";
    sprintf(file_spec, "/sdcard/spectrum/spectrum_%d.csv", count+1);

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

    return ESP_OK;
};

esp_err_t sctp_flash_save_curve(curve_t curve) {
    int check;
    char* dir_name = "/sdcard/curves";
    struct stat sb;

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
    vTaskDelay(2000 / portTICK_RATE_MS);
    // wavelength = 0 then empty

    for (int i=0; i<6; i++) {
        curves[i].id = i;

        if (i < 3) {
            curves[i].wavelength = 400 + i*50;
            curves[i].points = 5;
        }
        else if (i == 3) {
            curves[i].wavelength = 700;
            curves[i].points = 5;
        }
        else {
            curves[i].wavelength = 0;
            curves[i].points = 0;
        }

        strcpy(curves[i].filename, mockup_curves_filename[i]);
    }

    return ESP_OK;
}

esp_err_t sctp_flash_load_curve_floats(curve_t * curve) {
    for (int i=0; i < 15; i++) {
        curve->absorbance[i] = 0;
        curve->concentration[i] = 0;
        if (i < curve->points) {
            curve->absorbance[i] = (i+1) * 0.1;
            curve->concentration[i] = (i+1) * 0.001;
        }
    }

    return ESP_OK;
}

esp_err_t sctp_flash_load_history_list(char ** filenames) {
    char* dir_name = "/sdcard/spectrum";
    struct dirent *de;
    int i = 0;

    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        ESP_LOGI(TAG, "Could'nt open directory");
    }
    while ((de = readdir(dir)) != NULL) {
        filenames[i] = de->d_name;
        i++;
    }
    closedir(dir);
}

esp_err_t sctp_flash_load_spectrum(char * filename, float * absorbance, float * wavelength, uint16_t length) {
}