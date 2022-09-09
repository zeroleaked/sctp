#include "sctp.h"
#include "concrete_sctp_states.h"

// Constructor
Sctp::Sctp()
{
    currentState = &LightOff::getInstance();
	currentState->enter(this);
}

void Sctp::setState(SctpState& newState)
{
	currentState->exit(this);  // do stuff before we change state
	currentState = &newState;  // change state
	currentState->enter(this); // do stuff after we change state
}

void Sctp::okay()
{
	// Delegate the task of determining the next state to the current state!
	currentState->okay(this);
}

void Sctp::arrowDown()
{
	// Delegate the task of determining the next state to the current state!
	currentState->arrowDown(this);
}

int Sctp::getCurrentStateId()
{
    return currentState->id(this);
}