#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "sccb.h"
#include "mt9m001.h"
#include "mt9m001_regs.h"
#include "mt9m001_settings.h"

static const char *TAG = "MT9M001";

static int reset(sensor_t *sensor)
{
    int ret = 0;
    SCCB_Write(sensor->slv_addr, REG_RESET, 0x0001);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    SCCB_Write(sensor->slv_addr, REG_RESET, 0x0000);
    return ret;
}

static int init_status(sensor_t *sensor)
{
    uint16_t window_width = 0x0002;
    SCCB_Write(sensor->slv_addr, REG_HEIGHT, window_width);
    // uint16_t test_data = SCCB_Read(sensor->slv_addr, REG_WIDTH);
    // ESP_LOGD(TAG, "Reg0x04: 0x%04X", test_data);

    // test data
    // uint16_t octrl_status = SCCB_Read(sensor->slv_addr, REG_OCTRL);
    // SCCB_Write(sensor->slv_addr, REG_OCTRL, (1<<6) | octrl_status);
    // uint16_t test_data = SCCB_Read(sensor->slv_addr, REG_TEST_DATA);
    // SCCB_Write(sensor->slv_addr, REG_TEST_DATA, 0x3FF | test_data);
    // test_data = SCCB_Read8_16(sensor->slv_addr, REG_TEST_DATA);
    // ESP_LOGD(TAG, "Reg0x32: 0x%X", test_data);

    return 0;
}

static int set_test_data (sensor_t *sensor, uint16_t test_data) {
    uint16_t octrl_status = SCCB_Read(sensor->slv_addr, REG_OCTRL);
    SCCB_Write(sensor->slv_addr, REG_OCTRL, (1<<6) | octrl_status);
    uint16_t temp = SCCB_Read(sensor->slv_addr, REG_TEST_DATA);
    SCCB_Write(sensor->slv_addr, REG_TEST_DATA, test_data | temp);

    return 0;
}

// default row start = 0x000C
static int set_row_start (sensor_t *sensor, uint16_t row_start) {
    SCCB_Write(sensor->slv_addr, REG_RSTART, row_start);
    return 0;
}

// default row start = 0x000C
static int set_shutter_width (sensor_t *sensor, uint16_t shutter_width) {
    SCCB_Write(sensor->slv_addr, REG_SWIDTH, shutter_width);
    return 0;
}

static int set_gain(sensor_t *sensor, int gain)
{
    if(gain < 1) {
        gain = 1;
    } else if(gain > 15) {
        gain = 15;
    }

    uint16_t reg_gain;

    if (gain <= 4) {
        reg_gain = gain << 3;
    } else {
        reg_gain = (gain << 2) | 0x40;
    }

    ESP_LOGD(TAG, "reg_gain=0x%04x", reg_gain);
    int ret = SCCB_Write(sensor->slv_addr, REG_GLOB_G, reg_gain);
    uint16_t reg_status = SCCB_Read(sensor->slv_addr, REG_GLOB_G);
    ESP_LOGD(TAG, "Reg0x35: 0x%04X", reg_status);
    if (ret!= ESP_OK) {
        ESP_LOGE(TAG, "Gain setting failed");
        return -1;
    }
    return ret;
}

static int read_reg(sensor_t *sensor, uint8_t reg_addr, uint16_t * data)
{
    *data = SCCB_Read(sensor->slv_addr, reg_addr);
    return 1;
}

int mt9m001_init(sensor_t *sensor)
{
    sensor->reset = reset;
    sensor->init_status = init_status;
    sensor->set_test_data = set_test_data;
    sensor->set_row_start = set_row_start;
    sensor->set_shutter_width = set_shutter_width;
    sensor->set_gain = set_gain;
    sensor->read_reg = read_reg;
    // sensor->set_contrast  = set_dummy;
    // sensor->set_brightness= set_dummy;
    // sensor->set_saturation= set_dummy;

    // sensor->set_quality = set_dummy;
    // sensor->set_colorbar = set_dummy;

    // sensor->set_gain_ctrl = set_dummy;
    // sensor->set_exposure_ctrl = set_dummy;
    // sensor->set_hmirror = set_dummy;
    // sensor->set_vflip = set_dummy;

    // sensor->set_whitebal = set_dummy;
    // sensor->set_aec2 = set_dummy;
    // sensor->set_aec_value = set_dummy;
    // sensor->set_special_effect = set_dummy;
    // sensor->set_wb_mode = set_dummy;
    // sensor->set_ae_level = set_dummy;

    // sensor->set_dcw = set_dummy;
    // sensor->set_bpc = set_dummy;
    // sensor->set_wpc = set_dummy;
    // sensor->set_awb_gain = set_dummy;
    // sensor->set_agc_gain = set_agc_gain;

    // sensor->set_raw_gma = set_dummy;
    // sensor->set_lenc = set_dummy;

    // //not supported
    // sensor->set_sharpness = set_dummy;
    // sensor->set_denoise = set_dummy;

    // sensor->set_rowstart = set_rowstart;
    // sensor->set_colstart = set_colstart;
    // sensor->set_skip4 = set_skip4;
    // sensor->set_skip2 = set_skip2;

    ESP_LOGD(TAG, "MT9M001 Attached");
    return 0;
}