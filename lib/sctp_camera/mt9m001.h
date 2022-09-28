#ifndef __MT9M001_H__
#define __MT9M001_H__

#define MT9M001_SCCB_ADDR 0x5D

#include "sensor.h"

// /**
//  * @brief Detect sensor pid
//  *
//  * @param slv_addr SCCB address
//  * @param id Detection result
//  * @return
//  *     0:       Can't detect this sensor
//  *     Nonzero: This sensor has been detected
//  */
// int mt9m001_detect(int slv_addr, sensor_id_t *id);

/**
 * @brief initialize sensor function pointers
 *
 * @param sensor pointer of sensor
 * @return
 *      Always 0
 */
int mt9m001_init(sensor_t *sensor);

#endif