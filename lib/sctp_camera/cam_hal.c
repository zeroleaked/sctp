#include <string.h>
#include <esp_err.h>
#include <esp_check.h>

// #include <stdio.h>
// #include <string.h>
// #include "esp_heap_caps.h"
#include "ll_cam.h"
#include "cam_hal.h"

// #include "esp_timer.h"
// #include "esp32s3/rom/ets_sys.h"

static const char *TAG = "cam_hal";
static cam_obj_t *cam_obj = NULL;


esp_err_t cam_init(const camera_config_t *config)
{
    ESP_RETURN_ON_FALSE(config != NULL, ESP_ERR_INVALID_ARG, TAG, "config pointer is invalid");

    cam_obj = (cam_obj_t *)heap_caps_calloc(1, sizeof(cam_obj_t), MALLOC_CAP_DMA);
    ESP_RETURN_ON_FALSE(cam_obj != NULL, ESP_ERR_NO_MEM, TAG, "cam object malloc error");

    cam_obj->swap_data = 1;
    cam_obj->vsync_pin = config->pin_vsync;
    cam_obj->vsync_invert = true;

    ll_cam_set_pin(cam_obj, config);
    esp_err_t ret = ll_cam_config(cam_obj, config);
    ESP_RETURN_ON_ERROR(ret, TAG, "ll_cam initialize failed");

    ESP_LOGI(TAG, "cam init ok");
    return ESP_OK;
}

static lldesc_t * allocate_dma_descriptors(uint32_t count, uint16_t size, uint8_t * buffer)
{
    lldesc_t *dma = (lldesc_t *)heap_caps_malloc(count * sizeof(lldesc_t), MALLOC_CAP_DMA);
    if (dma == NULL) {
        return dma;
    }

    for (int x = 0; x < count; x++) {
        dma[x].size = size;
        dma[x].length = 0;
        dma[x].sosf = 0;
        dma[x].eof = 0;
        dma[x].owner = 1;
        dma[x].buf = (buffer + size * x);
        dma[x].empty = (uint32_t)&dma[(x + 1) % count];
    }
    return dma;
}

static esp_err_t cam_dma_config(const camera_config_t *config)
{
    bool ret = ll_cam_dma_sizes(cam_obj);
    if (0 == ret) {
        return ESP_FAIL;
    }

    cam_obj->dma_node_cnt = (cam_obj->dma_buffer_size) / cam_obj->dma_node_buffer_size; // Number of DMA nodes/decriptor
    cam_obj->frame_copy_cnt = cam_obj->recv_size / cam_obj->dma_half_buffer_size; // Number of interrupted copies, ping-pong copy

    ESP_LOGI(TAG, "buffer_size: %d, half_buffer_size: %d, node_buffer_size: %d, node_cnt: %d, total_cnt: %d", 
             cam_obj->dma_buffer_size, cam_obj->dma_half_buffer_size, cam_obj->dma_node_buffer_size, cam_obj->dma_node_cnt, cam_obj->frame_copy_cnt);

    cam_obj->dma_buffer = NULL;
    cam_obj->dma = NULL;

    cam_obj->frames = (cam_frame_t *)heap_caps_calloc(1, cam_obj->frame_cnt * sizeof(cam_frame_t), MALLOC_CAP_DEFAULT);
    ESP_RETURN_ON_FALSE(cam_obj->frames != NULL, ESP_ERR_NO_MEM, TAG, "frames malloc failed");

    size_t fb_size = cam_obj->fb_size;

    /* Allocate memory for frame buffer */
    size_t alloc_size = fb_size * sizeof(uint8_t);
    uint32_t _caps = MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL;

    for (int x = 0; x < cam_obj->frame_cnt; x++) {
        cam_obj->frames[x].dma = NULL;
        cam_obj->frames[x].fb_offset = 0;
        cam_obj->frames[x].en = 0;
        ESP_LOGI(TAG, "Allocating %d Byte frame buffer in %s", alloc_size, _caps & MALLOC_CAP_SPIRAM ? "PSRAM" : "OnBoard RAM");
        cam_obj->frames[x].fb.buf = (uint8_t *)heap_caps_malloc(alloc_size, _caps);
        ESP_RETURN_ON_FALSE(cam_obj->frames[x].fb.buf != NULL, ESP_ERR_NO_MEM, TAG, "frame buffer malloc failed");
        cam_obj->frames[x].en = 1;
    }

    cam_obj->dma_buffer = (uint8_t *)heap_caps_malloc(cam_obj->dma_buffer_size * sizeof(uint8_t), MALLOC_CAP_DMA);
    if(NULL == cam_obj->dma_buffer) {
        ESP_LOGE(TAG,"%s(%d): DMA buffer %d Byte malloc failed, the current largest free block:%d Byte", __FUNCTION__, __LINE__, 
                    cam_obj->dma_buffer_size, heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
        return ESP_ERR_NO_MEM;
    }

    cam_obj->dma = allocate_dma_descriptors(cam_obj->dma_node_cnt, cam_obj->dma_node_buffer_size, cam_obj->dma_buffer);
    ESP_RETURN_ON_FALSE(cam_obj->dma != NULL, ESP_ERR_NO_MEM, TAG, "dma malloc failed");

    return ESP_OK;
}

static bool cam_get_next_frame(int * frame_pos)
{
    if(!cam_obj->frames[*frame_pos].en){
        for (int x = 0; x < cam_obj->frame_cnt; x++) {
            if (cam_obj->frames[x].en) {
                *frame_pos = x;
                return true;
            }
        }
    } else {
        return true;
    }
    return false;
}

static bool cam_start_frame(int * frame_pos)
{
    if (cam_get_next_frame(frame_pos)) {
        if(ll_cam_start(cam_obj, *frame_pos)){
            // Vsync the frame manually
            ll_cam_do_vsync(cam_obj);
            uint64_t us = (uint64_t)esp_timer_get_time();
            cam_obj->frames[*frame_pos].fb.timestamp.tv_sec = us / 1000000UL;
            cam_obj->frames[*frame_pos].fb.timestamp.tv_usec = us % 1000000UL;
            return true;
        }
    }
    return false;
}


void cam_give(camera_fb_t *dma_buffer)
{
    for (int x = 0; x < cam_obj->frame_cnt; x++) {
        if (&cam_obj->frames[x].fb == dma_buffer) {
            cam_obj->frames[x].en = 1;
            break;
        }
    }
}


static void cam_task(void *arg)
{
    int cnt = 0;
    int frame_pos = 0;
    cam_obj->state = CAM_STATE_IDLE;
    cam_event_t cam_event = 0;
    
    xQueueReset(cam_obj->event_queue);

    while (1) {
        xQueueReceive(cam_obj->event_queue, (void *)&cam_event, portMAX_DELAY);
        switch (cam_obj->state) {

            case CAM_STATE_IDLE: {
                if (cam_event == CAM_VSYNC_EVENT) {
                    // start copying to available fb
                    if(cam_start_frame(&frame_pos)){
                        cam_obj->frames[frame_pos].fb.len = 0;
                        cam_obj->state = CAM_STATE_READ_BUF;
                    }
                    cnt = 0;
                }
            }
            break;

            case CAM_STATE_READ_BUF: {
                camera_fb_t * frame_buffer_event = &cam_obj->frames[frame_pos].fb;
                size_t pixels_per_dma = (cam_obj->dma_half_buffer_size * cam_obj->fb_bytes_per_pixel) / (cam_obj->dma_bytes_per_item * cam_obj->in_bytes_per_pixel);
                
                if (cam_event == CAM_IN_SUC_EOF_EVENT) {
                    // ESP_LOGD(TAG, "eof event");
                    if (cam_obj->fb_size < (frame_buffer_event->len + pixels_per_dma)) {
                        // ESP_LOGD(TAG, "%d < %d + %d", cam_obj->fb_size, frame_buffer_event->len, pixels_per_dma);
                        
                        // for (int i=0; i < (frame_buffer_event->len / 2); i++) {
                        //     uint8_t HREAD = frame_buffer_event->buf[i*2];
                        //     uint8_t LREAD = frame_buffer_event->buf[i*2 + 1];
                        //     if ( !((HREAD==0 && LREAD==0) || (HREAD==0xFF && LREAD==0x03)) ) {
                        //         ESP_LOGD(TAG, "%d: %02x %02x", i, HREAD, LREAD);
                        //         break;
                        //     }
                        // }

                        // uint8_t peek[16];
                        // for (int i=0; i<16; i++) {
                        //     peek[i] = frame_buffer_event->buf[i];
                        //     // peek[i] = frame_buffer_event->buf[frame_buffer_event->len -16 + i];
                        // }

                        // ESP_LOGD(TAG, "%x, %x, %x, %x, %x, %x, %x, %x", peek[0], peek[1], peek[2], peek[3], peek[4], peek[5], peek[6], peek[7]);
                        // ESP_LOGD(TAG, "%x, %x, %x, %x, %x, %x, %x, %x", peek[8], peek[9], peek[10], peek[11], peek[12], peek[13], peek[14], peek[15]);
                        ESP_LOGW(TAG, "FB-OVF");
                        ll_cam_stop(cam_obj);
                        continue;
                    }
                    memcpy(
                        &frame_buffer_event->buf[frame_buffer_event->len], 
                        &cam_obj->dma_buffer[(cnt % cam_obj->dma_half_buffer_cnt) * cam_obj->dma_half_buffer_size], 
                        cam_obj->dma_half_buffer_size);
                    frame_buffer_event->len += cam_obj->dma_half_buffer_size;
                    cnt++;

                } else if (cam_event == CAM_VSYNC_EVENT) {
                    // ESP_LOGD(TAG, "vsync event");
                    //DBG_PIN_SET(1);
                    ll_cam_stop(cam_obj);

                    cam_obj->frames[frame_pos].en = 0;

                    if (frame_buffer_event->len != cam_obj->fb_size) {
                        cam_obj->frames[frame_pos].en = 1;
                        ESP_LOGE(TAG, "FB-SIZE: %u != %u", frame_buffer_event->len, cam_obj->fb_size);
                    }
                    //send frame
                    if(!cam_obj->frames[frame_pos].en && xQueueSend(cam_obj->frame_buffer_queue, (void *)&frame_buffer_event, 0) != pdTRUE) {
                        // ESP_LOGD(TAG, "full queue, popping...");
                        // if current frame dma is finished but fb queue is full
                        // pop old frame buffer from the queue
                        camera_fb_t * fb2 = NULL;
                        if(xQueueReceive(cam_obj->frame_buffer_queue, &fb2, 0) == pdTRUE) {
                            //push the new frame to the end of the queue
                            if (xQueueSend(cam_obj->frame_buffer_queue, (void *)&frame_buffer_event, 0) != pdTRUE) {
                                cam_obj->frames[frame_pos].en = 1;
                                ESP_LOGE(TAG, "FBQ-SND");
                            }
                            //free the popped buffer
                            cam_give(fb2);
                            // ESP_LOGD(TAG, "popped");
                        } else {
                            //queue is full and we could not pop a frame from it
                            cam_obj->frames[frame_pos].en = 1;
                            ESP_LOGE(TAG, "FBQ-RCV");
                        }
                    }
                    // else if (!cam_obj->frames[frame_pos].en) ESP_LOGD(TAG, "added to queue");

                    if(!cam_start_frame(&frame_pos)){
                        cam_obj->state = CAM_STATE_IDLE;
                    } else {
                        cam_obj->frames[frame_pos].fb.len = 0;
                    }
                    cnt = 0;
                }
            }
            break;
        }
    }
}

esp_err_t cam_config(const camera_config_t *config)
{
    ESP_RETURN_ON_FALSE(config != NULL, ESP_ERR_INVALID_ARG, TAG, "config pointer is invalid");

    esp_err_t ret = ESP_OK;

    ll_cam_set_sample_mode(cam_obj);

    cam_obj->frame_cnt = config->fb_count;
    cam_obj->width = 1280;
    cam_obj->height = 3;

    cam_obj->recv_size = cam_obj->width * cam_obj->height * cam_obj->in_bytes_per_pixel;
    cam_obj->fb_size = cam_obj->width * cam_obj->height * cam_obj->fb_bytes_per_pixel;
    
    ret = cam_dma_config(config);
    ESP_RETURN_ON_ERROR(ret, TAG, "cam_dma_config failed");

    ESP_LOGI(TAG, "dma_half_buffer_cnt=%d", cam_obj->dma_half_buffer_cnt);
    cam_obj->event_queue = xQueueCreate(cam_obj->dma_half_buffer_cnt - 1, sizeof(cam_event_t));
    ESP_RETURN_ON_FALSE(cam_obj->event_queue != NULL, ESP_FAIL, TAG, "event_queue create failed");

    size_t frame_buffer_queue_len = cam_obj->frame_cnt;
    if (cam_obj->frame_cnt > 1) {
        frame_buffer_queue_len = cam_obj->frame_cnt - 1;
    }
    ESP_LOGI(TAG, "frame_buffer_queue_len=%d", frame_buffer_queue_len);
    cam_obj->frame_buffer_queue = xQueueCreate(frame_buffer_queue_len, sizeof(camera_fb_t*));
    ESP_RETURN_ON_FALSE(cam_obj->frame_buffer_queue != NULL, ESP_FAIL, TAG, "frame_buffer_queue create failed");

    ret = ll_cam_init_isr(cam_obj);
    ESP_RETURN_ON_ERROR(ret, TAG, "cam intr alloc failed");

    xTaskCreate(cam_task, "cam_task", 2048, NULL, configMAX_PRIORITIES - 2, &cam_obj->task_handle);

    ESP_LOGI(TAG, "cam config ok");
    return ESP_OK;

// err:
//     cam_deinit();
//     return ESP_FAIL;
}

void cam_start(void)
{
    ll_cam_vsync_intr_enable(cam_obj, true);
}

camera_fb_t *cam_take(TickType_t timeout)
{
    camera_fb_t *dma_buffer = NULL;
    xQueueReceive(cam_obj->frame_buffer_queue, (void *)&dma_buffer, timeout);
    if (dma_buffer) {
        return dma_buffer;
    } else {
        ESP_LOGW(TAG, "Failed to get the frame on time!");
    }
    return NULL;
}

void cam_stop(void)
{
    ll_cam_vsync_intr_enable(cam_obj, false);
    ll_cam_stop(cam_obj);
}

esp_err_t cam_deinit(void)
{
    if (!cam_obj) {
        return ESP_FAIL;
    }

    cam_stop();
    if (cam_obj->task_handle) {
        vTaskDelete(cam_obj->task_handle);
    }
    if (cam_obj->event_queue) {
        vQueueDelete(cam_obj->event_queue);
    }
    if (cam_obj->frame_buffer_queue) {
        vQueueDelete(cam_obj->frame_buffer_queue);
    }
    if (cam_obj->dma) {
        free(cam_obj->dma);
    }
    if (cam_obj->dma_buffer) {
        free(cam_obj->dma_buffer);
    }
    if (cam_obj->frames) {
        for (int x = 0; x < cam_obj->frame_cnt; x++) {
            free(cam_obj->frames[x].fb.buf - cam_obj->frames[x].fb_offset);
            if (cam_obj->frames[x].dma) {
                free(cam_obj->frames[x].dma);
            }
        }
        free(cam_obj->frames);
    }

    ll_cam_deinit(cam_obj);

    free(cam_obj);
    cam_obj = NULL;
    return ESP_OK;
}