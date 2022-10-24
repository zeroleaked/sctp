#include <math.h>
#include <esp_log.h>

#include <i2cdev.h>

#include "sctp_common_types.h"
#include "sctp_battery.h"

const static char TAG[] = "sctp_battery";

static i2c_dev_t i2c_dev;

uint16_t get_config() {
    uint8_t reg = 0x00;
    uint16_t in_data = 0;
    i2c_dev_read(&i2c_dev, &reg, 1, &in_data, 2);

    uint16_t byte_flip = ((in_data & 0x00FF) << 8) | ((in_data & 0xFF00) >> 8); 

    return byte_flip;

}

void setCalibration_32V_2A() {
  uint32_t ina219_calValue = 4096;

  // Set multipliers to convert raw current/power values
  uint32_t ina219_currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)
  float ina219_powerMultiplier_mW = 2; // Power LSB = 1mW per bit (2/1)

  // Set Calibration register to 'Cal' calculated above
  uint8_t reg = 0x05;
  uint16_t byte_flip = ((ina219_calValue & 0x00FF) << 8) | ((ina219_calValue & 0xFF00) >> 8); 
  i2c_dev_write(&i2c_dev, &reg, 1, &byte_flip, 2);

  // Set Config register to take into account the settings above
  reg = 0x00;
  uint16_t config = 0x2000 |
                    0x1800 | 0x0180 |
                    0x0018 |
                    0x07;

  byte_flip = ((config & 0x00FF) << 8) | ((config & 0xFF00) >> 8);
  esp_err_t ret = i2c_dev_write(&i2c_dev, &reg, 1, &byte_flip, 2);

  get_config();
}

esp_err_t sctp_battery_init() {
    i2c_dev.port = SCTP_I2C_PORT;
    i2c_dev.addr = 0x40;
    i2c_dev.cfg.sda_io_num = SCTP_I2C_SDA;
    i2c_dev.cfg.scl_io_num = SCTP_I2C_SCL;
    i2c_dev.cfg.master.clk_speed = 100000;

    setCalibration_32V_2A();

    return ESP_OK;
}

int16_t get_voltage() {
    uint8_t reg = 0x02;
    uint16_t in_data = 0;
    i2c_dev_read(&i2c_dev, &reg, 1, &in_data, 2);

    uint16_t byte_flip = ((in_data & 0x00FF) << 8) | ((in_data & 0xFF00) >> 8); 

    return (int16_t)((byte_flip >> 3) * 4);

}

esp_err_t sctp_battery_sample(uint8_t * percentage) {
    get_config();

    float volt_bat = (float) get_voltage()/2000;

    ESP_LOGI(TAG, "volt_bat=%f", volt_bat);

    *percentage = 268.497 * pow(volt_bat,8) + -6879.270 * pow(volt_bat,7) + 76716.496 * pow(volt_bat,6) - 486365.673 * pow(volt_bat,5) + 1917287.171 * pow(volt_bat,4) - 4812471.066 * pow(volt_bat,3) + 7511312.301 * pow(volt_bat,2) - (6665390.783 * volt_bat) + 2574719.230;
    if (volt_bat <= 2.6) {
        *percentage = 0;
    }
    if (volt_bat >=4.0){
        *percentage = 100;
    }
    return ESP_OK;
}