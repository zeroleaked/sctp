#pragma once

#include "sctp_state.h"

class SctpState; // forward declaration

class Sctp
{
public:
	Sctp();
	// Same as before
	inline SctpState* getCurrentState() const { return currentState; }
	// In here, we'll delegate the state transition to the currentState
	void toggle();

	void okay();

	void arrowDown();

	// This will get called by the current state
	void setState(SctpState& newState);


    int getCurrentStateId();

	int cursor;

private:
	// LightState here is now a class, not the enum that we saw earlier
	SctpState* currentState;
};