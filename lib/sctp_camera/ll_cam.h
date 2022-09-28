#pragma once

// #include <stdint.h>
// #include "sdkconfig.h"
// #include "esp_idf_version.h"
#include <esp32s3/rom/lldesc.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
// #include "freertos/task.h"
// #include "freertos/semphr.h"

#include "sctp_camera.h"

#define LCD_CAM_DMA_NODE_BUFFER_MAX_SIZE  (4092)

typedef enum {
    CAM_IN_SUC_EOF_EVENT = 0,
    CAM_VSYNC_EVENT
} cam_event_t;

typedef struct {
    camera_fb_t fb;
    uint8_t en;
    //for RGB/YUV modes
    lldesc_t *dma;
    size_t fb_offset;
} cam_frame_t;

typedef enum {
    CAM_STATE_IDLE = 0,
    CAM_STATE_READ_BUF = 1,
} cam_state_t;

typedef struct {
    uint32_t dma_bytes_per_item;
    uint32_t dma_buffer_size;
    uint32_t dma_half_buffer_size;
    uint32_t dma_half_buffer_cnt;
    uint32_t dma_node_buffer_size;
    uint32_t dma_node_cnt;
    uint32_t frame_copy_cnt;

    lldesc_t *dma;
    uint8_t  *dma_buffer;

    cam_frame_t *frames;

    QueueHandle_t event_queue;
    QueueHandle_t frame_buffer_queue;
    TaskHandle_t task_handle;
    intr_handle_t cam_intr_handle;
	
    uint8_t dma_num; //GDMA CHANNEL
    intr_handle_t dma_intr_handle;//ESP32-S3

    uint8_t vsync_pin;
    uint8_t vsync_invert;
    uint32_t frame_cnt;
    uint32_t recv_size;
    bool swap_data;

    //for RGB/YUV modes
    uint16_t width;
    uint16_t height;

    uint8_t in_bytes_per_pixel;
    uint8_t fb_bytes_per_pixel;

    uint32_t fb_size;

    cam_state_t state;
} cam_obj_t;

esp_err_t ll_cam_set_pin(cam_obj_t *cam, const camera_config_t *config);
esp_err_t ll_cam_config(cam_obj_t *cam, const camera_config_t *config);
void ll_cam_vsync_intr_enable(cam_obj_t *cam, bool en);
esp_err_t ll_cam_set_sample_mode(cam_obj_t *cam);
esp_err_t ll_cam_init_isr(cam_obj_t *cam);
bool ll_cam_start(cam_obj_t *cam, int frame_pos);
bool ll_cam_stop(cam_obj_t *cam);
void ll_cam_do_vsync(cam_obj_t *cam);
bool ll_cam_dma_sizes(cam_obj_t *cam);
esp_err_t ll_cam_deinit(cam_obj_t *cam);