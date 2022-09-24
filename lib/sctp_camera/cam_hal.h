#pragma once

#include "sctp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t cam_init(const camera_config_t *config);
// esp_err_t cam_deinit(void);
esp_err_t cam_config(const camera_config_t *config);
void cam_start(void);
camera_fb_t *cam_take(TickType_t timeout);
void cam_give(camera_fb_t *dma_buffer);
esp_err_t cam_deinit(void);

#ifdef __cplusplus
}
#endif
