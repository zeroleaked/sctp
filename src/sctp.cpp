#include "sctp.h"
#include "concrete_sctp_states.h"

// Constructor
Sctp::Sctp()
{
    currentState = &LightOff::getInstance();
}

void Sctp::setState(SctpState& newState)
{
	currentState->exit(this);  // do stuff before we change state
	currentState = &newState;  // change state
	currentState->enter(this); // do stuff after we change state
}

void Sctp::toggle()
{
	// Delegate the task of determining the next state to the current state!
	currentState->toggle(this);
}

int Sctp::getCurrentStateId()
{
    return currentState->id(this);
}