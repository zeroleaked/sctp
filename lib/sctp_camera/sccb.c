/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"

#include <i2cdev.h>

#include "sccb.h"
#include "sensor.h"
static const char* TAG = "sccb";

#define LITTLETOBIG(x)          ((x<<8)|(x>>8))

#include "driver/i2c.h"

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#define SCCB_FREQ               100000  /*!< I2C master frequency*/
#define WRITE_BIT               I2C_MASTER_WRITE      /*!< I2C master write */
#define READ_BIT                I2C_MASTER_READ       /*!< I2C master read */
#define ACK_CHECK_EN            0x1                   /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS           0x0                   /*!< I2C master will not check ack from slave */
#define ACK_VAL                 0x0                   /*!< I2C ack value */
#define NACK_VAL                0x1                   /*!< I2C nack value */
#if CONFIG_SCCB_HARDWARE_I2C_PORT1
const int SCCB_I2C_PORT         = 1;
#else
const int SCCB_I2C_PORT         = 0;
#endif

static i2c_dev_t i2c_dev;

int SCCB_Init(int pin_sda, int pin_scl)
{
    ESP_LOGI(TAG, "pin_sda %d pin_scl %d", pin_sda, pin_scl);

    i2c_dev.port = SCCB_I2C_PORT;
    i2c_dev.addr = 0x5D; // mt9m001 slave address
    i2c_dev.cfg.sda_io_num = pin_sda;
    i2c_dev.cfg.scl_io_num = pin_scl;
    i2c_dev.cfg.master.clk_speed = SCCB_FREQ;
    return 0;
}

int SCCB_Deinit(void) {
    return ESP_OK;

}

uint16_t SCCB_Read(uint8_t slv_addr, uint8_t reg)
{
    uint16_t in_data = 0;
    i2c_dev_read(&i2c_dev, &reg, 1, &in_data, 2);

    uint16_t byte_flip = ((in_data & 0x00FF) << 8) | ((in_data & 0xFF00) >> 8); 
    return byte_flip;
}

uint8_t SCCB_Write(uint8_t slv_addr, uint8_t reg, uint16_t data)
{
    uint16_t byte_flip = ((data & 0x00FF) << 8) | ((data & 0xFF00) >> 8); 
    esp_err_t ret = i2c_dev_write(&i2c_dev, &reg, 1, &byte_flip, 2);
    return ret;
}