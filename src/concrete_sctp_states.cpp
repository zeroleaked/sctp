#include "concrete_sctp_states.h"

void LightOff::toggle(Sctp* sctp)
{
	// Off -> Low
	sctp->setState(LowIntensity::getInstance());
}

int LightOff::id(Sctp* sctp)
{
	return 1;
}

SctpState& LightOff::getInstance()
{
	static LightOff singleton;
	return singleton;
}

void LowIntensity::toggle(Sctp* sctp)
{
	// Low -> Medium
	sctp->setState(MediumIntensity::getInstance());
}

int LowIntensity::id(Sctp* sctp)
{
	return 2;
}

SctpState& LowIntensity::getInstance()
{
	static LowIntensity singleton;
	return singleton;
}

void MediumIntensity::toggle(Sctp* sctp)
{
	// Medium -> High
	sctp->setState(HighIntensity::getInstance());
}

int MediumIntensity::id(Sctp* sctp)
{
	return 3;
}

SctpState& MediumIntensity::getInstance()
{
	static MediumIntensity singleton;
	return singleton;
}

void HighIntensity::toggle(Sctp* sctp)
{
	// High -> Low
	sctp->setState(LightOff::getInstance());
}

int HighIntensity::id(Sctp* sctp)
{
	return 4;
}

SctpState& HighIntensity::getInstance()
{
	static HighIntensity singleton;
	return singleton;
}