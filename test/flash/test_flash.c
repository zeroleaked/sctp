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

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  GPIO_NUM_12
#define PIN_NUM_MOSI  GPIO_NUM_13
#define PIN_NUM_CLK   GPIO_NUM_14
#define PIN_NUM_CS    GPIO_NUM_15

#define NAME_LEN 25

typedef struct {
	double gain;
	double bias;

	uint16_t row;
	uint16_t start; // pixel column index
	uint16_t length; // length of full spectrum
} calibration_t;

sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdmmc_card_t * card;
const char mount_point[] = MOUNT_POINT;

void sctp_flash_init(gpio_num_t cs_gpio) {
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

    //sdmmc_host_t host = SDSPI_HOST_DEFAULT();

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
    slot_config.gpio_cs = cs_gpio;
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

void sctp_flash_deinit() {
    //sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free((spi_host_device_t)host.slot);
}

esp_err_t sctp_flash_save_calibration(calibration_t data, char * filename) {
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

    return ESP_OK;
}

esp_err_t sctp_flash_load_calibration(calibration_t * data, char * filename) {
    char line[NAME_LEN];
    char file_cal[NAME_LEN];
    strcpy(file_cal, "/sdcard/");
    strcat(file_cal, filename);
    char *temp;

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
    return ESP_OK;
}

void test1(void)
{
//     esp_err_t ret;
//     gpio_pullup_en(PIN_NUM_MOSI);

//     // Options for mounting the filesystem.
//     // If format_if_mount_failed is set to true, SD card will be partitioned and
//     // formatted in case when mounting fails.
//     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
// #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
//         .format_if_mount_failed = true,
// #else
//         .format_if_mount_failed = false,
// #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
//         .max_files = 5,
//         .allocation_unit_size = 16 * 1024
//     };
//     sdmmc_card_t *card;
//     const char mount_point[] = MOUNT_POINT;
//     ESP_LOGI(TAG, "Initializing SD card");

//     // Use settings defined above to initialize SD card and mount FAT filesystem.
//     // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
//     // Please check its source code and implement error recovery when developing
//     // production applications.
//     ESP_LOGI(TAG, "Using SPI peripheral");

//     // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
//     // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
//     // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
//     sdmmc_host_t host = SDSPI_HOST_DEFAULT();

//     spi_bus_config_t bus_cfg = {
//         .mosi_io_num = PIN_NUM_MOSI,
//         .miso_io_num = PIN_NUM_MISO,
//         .sclk_io_num = PIN_NUM_CLK,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//         .max_transfer_sz = 4000,
//     };
//     ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to initialize bus.");
//         return;
//     }

//     // This initializes the slot without card detect (CD) and write protect (WP) signals.
//     // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
//     sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
//     slot_config.gpio_cs = PIN_NUM_CS;
//     slot_config.host_id = host.slot;

//     ESP_LOGI(TAG, "Mounting filesystem");
//     ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

//     if (ret != ESP_OK) {
//         if (ret == ESP_FAIL) {
//             ESP_LOGE(TAG, "Failed to mount filesystem. "
//                      "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
//         } else {
//             ESP_LOGE(TAG, "Failed to initialize the card (%s). "
//                      "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
//         }
//         return;
//     }
//     ESP_LOGI(TAG, "Filesystem mounted");

//     // Card has been initialized, print its properties
//     sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.
    sctp_flash_init(PIN_NUM_CS);

    calibration_t calibration;
    calibration_t calibration_load;
    float gain = 0;
	float bias = 0;
	uint16_t row = 0;
	uint16_t start = 0; // pixel column index
	uint16_t length = 0; // length of full spectrum
    calibration_load.gain = gain;
    calibration_load.bias = bias;
    calibration_load.row = row;
    calibration_load.start = start;
    calibration_load.length = length;
    char * calibration_file;
    calibration_file = malloc(sizeof(char)*NAME_LEN);
    calibration.row = 496;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;

    sctp_flash_save_calibration(calibration, calibration_file);
    sctp_flash_load_calibration(&calibration_load, calibration_file);

    ESP_LOGI(TAG, "%.10f", calibration_load.gain);
    ESP_LOGI(TAG, "%.10f", calibration_load.bias);
    ESP_LOGI(TAG, "%d", calibration_load.row);
    ESP_LOGI(TAG, "%d", calibration_load.start);
    ESP_LOGI(TAG, "%d", calibration_load.length);

    sctp_flash_deinit();

    // First create a file.
    // const char *file_hello = MOUNT_POINT"/hello.txt";

    // ESP_LOGI(TAG, "Opening file %s", file_hello);
    // FILE *f = fopen(file_hello, "w");
    // if (f == NULL) {
    //     ESP_LOGE(TAG, "Failed to open file for writing");
    //     return;
    // }
    // fprintf(f, "Hello %s!\n", card->cid.name);
    // fclose(f);
    // ESP_LOGI(TAG, "File written");

    // const char *file_foo = MOUNT_POINT"/foo.txt";

    // // Check if destination file exists before renaming
    // struct stat st;
    // if (stat(file_foo, &st) == 0) {
    //     // Delete it if it exists
    //     unlink(file_foo);
    // }

    // // Rename original file
    // ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    // if (rename(file_hello, file_foo) != 0) {
    //     ESP_LOGE(TAG, "Rename failed");
    //     return;
    // }

    // // Open renamed file for reading
    // ESP_LOGI(TAG, "Reading file %s", file_foo);
    // f = fopen(file_foo, "r");
    // if (f == NULL) {
    //     ESP_LOGE(TAG, "Failed to open file for reading");
    //     return;
    // }

    // // Read a line from file
    // char line[64];
    // fgets(line, sizeof(line), f);
    // fclose(f);

    // // Strip newline
    // char *pos = strchr(line, '\n');
    // if (pos) {
    //     *pos = '\0';
    // }
    // ESP_LOGI(TAG, "Read from file: '%s'", line);

    // // All done, unmount partition and disable SPI peripheral
    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // ESP_LOGI(TAG, "Card unmounted");

    // //deinitialize the bus after all devices are removed
    // spi_bus_free(host.slot);
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test1);
    UNITY_END();
}