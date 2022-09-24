#include <driver/ledc.h>
#include <esp_check.h>

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "time.h"
// #include "sys/time.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #include "driver/gpio.h"
// #include "esp_system.h"
#include <esp_log.h>
// #include "sensor.h"
#include "sccb.h"
#include "cam_hal.h"
#include "sctp_camera.h"
#include "xclk.h"
#include "mt9m001.h"

static const char *TAG = "camera";

typedef struct {
    sensor_t sensor;
    camera_fb_t fb;
} camera_state_t;

static camera_state_t *s_state = NULL;

esp_err_t camera_probe(const camera_config_t *config)
{
    if (s_state != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    s_state = (camera_state_t *) calloc(sizeof(camera_state_t), 1);
    if (!s_state) {
        return ESP_ERR_NO_MEM;
    }

    if (config->pin_xclk >= 0) {
        ESP_LOGD(TAG, "Enabling XCLK output");
        camera_enable_out_clock(config);
    }

    if (config->pin_sscb_sda != -1) {
        ESP_LOGD(TAG, "Initializing SSCB");
        SCCB_Init(config->pin_sscb_sda, config->pin_sscb_scl);
    }

    if (config->pin_reset >= 0) {
        ESP_LOGD(TAG, "Resetting camera");
        gpio_config_t conf = { 0 };
        conf.pin_bit_mask = 1LL << config->pin_reset;
        conf.mode = GPIO_MODE_OUTPUT;
        gpio_config(&conf);

        gpio_set_level(config->pin_reset, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(config->pin_reset, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }


    ESP_LOGD(TAG, "Checking camera");
    vTaskDelay(10 / portTICK_PERIOD_MS);

    s_state->sensor.slv_addr = MT9M001_SCCB_ADDR;
    s_state->sensor.xclk_freq_hz = config->xclk_freq_hz;

    uint16_t id = SCCB_Read(MT9M001_SCCB_ADDR, 0x00);

    ESP_RETURN_ON_FALSE(id == 0x8431, ESP_ERR_NOT_FOUND, TAG, "mt9m001 register read fail");
    
    ESP_LOGI(TAG, "Detected MT9M001 camera");

    mt9m001_init(&s_state->sensor);

    ESP_LOGD(TAG, "Doing SW reset of sensor");
    vTaskDelay(10 / portTICK_PERIOD_MS);
    s_state->sensor.reset(&s_state->sensor);

    return ESP_OK;
}

esp_err_t sctp_camera_init(const camera_config_t *config)
{
    esp_err_t err;
    err = cam_init(config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    err = camera_probe(config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x(%s)", err, esp_err_to_name(err));
//     esp_camera_deinit();
        return err;
    }

    err = cam_config(config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera config failed with error 0x%x", err);
    }

    ESP_LOGD(TAG, "Setting frame size to 1280x3");
    s_state->sensor.init_status(&s_state->sensor);

    cam_start();

    return ESP_OK;

// fail:
//     esp_camera_deinit();
//     return err;
}

camera_fb_t *sctp_camera_fb_get()
{
    if (s_state == NULL) {
        return NULL;
    }
    camera_fb_t *fb = cam_take(4000 / portTICK_PERIOD_MS);
    //set the frame properties
    if (fb) {
        fb->width = 1280;
        fb->height = 3;
    }
    return fb;
}

sensor_t *sctp_camera_sensor_get()
{
    if (s_state == NULL) {
        return NULL;
    }
    return &s_state->sensor;
}

void sctp_camera_fb_return(camera_fb_t *fb)
{
    if (s_state == NULL) {
        return;
    }
    cam_give(fb);
}

esp_err_t sctp_camera_deinit()
{
    esp_err_t ret = cam_deinit();
    camera_disable_out_clock();
    if (s_state) {
        SCCB_Deinit();

        free(s_state);
        s_state = NULL;
    }

    return ret;
}