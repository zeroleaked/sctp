#pragma once

#include <stdint.h>

typedef enum {
	SPECTRUM_BLANK,
	SPECTRUM_SAMPLE
} command_t;

typedef struct {
	float * absorbance;
	float * concentration;
	uint16_t wavelength;
	uint8_t points;
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

	uint16_t start; // pixel column index
	uint16_t length; // length of full spectrum
} calibration_t;

typedef struct {
	float * readout;
	uint16_t exposure;
	uint16_t gain;
} blank_sample_t;