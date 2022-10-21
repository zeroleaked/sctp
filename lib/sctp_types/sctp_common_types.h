#pragma once

#include <stdint.h>

#define SCTP_I2C_PORT 0
#define SCTP_I2C_SCL GPIO_NUM_10
#define SCTP_I2C_SDA GPIO_NUM_11 

typedef enum {
	COMMAND_NONE,
	COMMAND_BAT_UPDATE
} command_t;

typedef struct {
	char * filename; // curve1_400nm.csv\0 (17 char) 
	// char filename[20]; // curve1_400nm.csv\0 (17 char) 
	uint16_t wavelength;
	uint8_t id;
	uint8_t points;
	float * absorbance;
	float * concentration;
} curve_t;

#define MEASUREMENT_MODE_SPECTRUM 0
#define MEASUREMENT_MODE_CONCENTRATION 1
typedef struct {
	uint8_t id;
	uint8_t measurement_mode;

	// concentration mode
	uint16_t wavelength;
	float absorbance;
	float concentration;
	uint8_t curve_id;

	// spectrum mode
	float * absorbance_buffer;
	
} history_t; // 20 bytes

typedef struct {
	float gain;
	float bias;

	uint16_t row;
	uint16_t start; // pixel column index
	uint16_t length; // length of full spectrum
} calibration_t;

typedef struct {
	float * readout;
	uint16_t * exposure;
	uint16_t gain;
} blank_take_t;

typedef struct {
	float absorbance;
	float concentration;
} conc_regression_point_t;

typedef struct {
	float gradient;
	float offset;
	conc_regression_point_t points[2];
} conc_regression_t;
