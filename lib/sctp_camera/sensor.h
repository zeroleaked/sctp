#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <stdint.h>
// #include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _sensor sensor_t;
typedef struct _sensor {
    uint8_t  slv_addr;          // Sensor I2C slave address.
    // camera_status_t status;
    int xclk_freq_hz;

    // Sensor function pointers
    int  (*init_status)         (sensor_t *sensor);
    int  (*reset)               (sensor_t *sensor);
    int  (*set_test_data)       (sensor_t *sensor, uint16_t test_data);
    int  (*set_row_start)       (sensor_t *sensor, uint16_t row_start);
    int  (*set_shutter_width)   (sensor_t *sensor, uint16_t shutter_width);
    int  (*set_gain)            (sensor_t *sensor, int gain);
    int  (*read_reg)            (sensor_t *sensor, uint8_t reg_addr, uint16_t * data);
    // int  (*set_hmirror)         (sensor_t *sensor, int enable);
    // int  (*set_vflip)           (sensor_t *sensor, int enable);

    // int  (*set_agc_gain)        (sensor_t *sensor, int gain);
    // int  (*set_aec_value)       (sensor_t *sensor, int gain);

    // int  (*get_reg)             (sensor_t *sensor, uint8_t reg, uint8_t mask);
    // int  (*set_reg)             (sensor_t *sensor, int reg, int mask, int value);
    // int  (*set_res_raw)         (sensor_t *sensor, int startX, int startY, int endX, int endY, int offsetX, int offsetY, int totalX, int totalY, int outputX, int outputY, bool scale, bool binning);
    // int  (*set_pll)             (sensor_t *sensor, int bypass, int mul, int sys, int root, int pre, int seld5, int pclken, int pclk);
    // int  (*set_xclk)            (sensor_t *sensor, int timer, int xclk);

    // int  (*set_rowstart)       (sensor_t *sensor, uint32_t row);
    // int  (*set_skip2)          (sensor_t *sensor, int enable);
    // int  (*set_skip4)          (sensor_t *sensor, int enable);
} sensor_t;

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_H__ */
