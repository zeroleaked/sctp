#pragma once

#include <stdint.h>

#include "sctp_state.h"

class SctpState; // forward declaration

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

class Sctp
{
public:
	Sctp();
	// Same as before
	inline SctpState* getCurrentState() const { return currentState; }
	// In here, we'll delegate the state transition to the currentState
	void toggle();

	// button OK
	void okay();

	// button arrow up
	void arrowUp();

	// button arrow down
	void arrowDown();

	// button arrow left
	void arrowLeft();

	// button arrow right
	void arrowRight();

	// This will get called by the current state
	void setState(SctpState& newState);


    int getCurrentStateId();

	calibration_t calibration;

	uint16_t wavelength;
	curve_t * curve; // max 6!
	uint8_t curve_length = 0;
	
	history_t * history_list;
	uint8_t history_list_length;
	uint8_t history_index;
	history_t history;


private:
	// LightState here is now a class, not the enum that we saw earlier
	SctpState* currentState;
};