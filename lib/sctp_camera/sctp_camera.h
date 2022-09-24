#pragma once

#include <esp_err.h>
#include <driver/ledc.h>
#include <sys/time.h>
#include <sdkconfig.h>

#include "sensor.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // GPIO config
    int pin_reset;
    int pin_xclk;                   
    int pin_sscb_sda;               
    int pin_sscb_scl;               
    int pin_d9;                     
    int pin_d8;                     
    int pin_d7;                     
    int pin_d6;                     
    int pin_d5;                     
    int pin_d4;                     
    int pin_d3;                     
    int pin_d2;                     
    int pin_d1;                     
    int pin_d0;                     
    int pin_vsync;                  
    int pin_href;                   
    int pin_pclk;                   

    // XCLK config
    int xclk_freq_hz;
    ledc_timer_t ledc_timer;
    ledc_channel_t ledc_channel;

    size_t fb_count;
} camera_config_t;

typedef struct {
    uint8_t * buf;              /*!< Pointer to the pixel data */
    size_t len;                 /*!< Length of the buffer in bytes */
    size_t width;               /*!< Width of the buffer in pixels */
    size_t height;              /*!< Height of the buffer in pixels */
    struct timeval timestamp;   /*!< Timestamp since boot of the first DMA buffer of the frame */
} camera_fb_t;

esp_err_t sctp_camera_init(const camera_config_t* config);
camera_fb_t* sctp_camera_fb_get();
sensor_t * sctp_camera_sensor_get();
void sctp_camera_fb_return(camera_fb_t * fb);
esp_err_t sctp_camera_deinit();

#ifdef __cplusplus
}
#endif

