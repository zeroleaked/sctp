#pragma once

#include <stdint.h>

#include "sctp_state.h"

class SctpState; // forward declaration

typedef struct {
	float * absorbance;
	float * concentration;
	uint8_t points;
	uint16_t wavelength;
} curve_t;

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

	uint16_t wavelength;
	curve_t * curve; // max 6!
	uint8_t curve_length = 0;


private:
	// LightState here is now a class, not the enum that we saw earlier
	SctpState* currentState;
};