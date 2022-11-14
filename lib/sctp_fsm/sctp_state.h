#pragma once

#include <sctp.h>
#include "sctp_common_types.h"

// #include <stdint.h>

class Sctp;
// Forward declaration to resolve circular dependency/include
class SctpState
{
public:
	virtual void enter(Sctp* sctp) = 0;
	virtual void okay(Sctp* sctp) = 0;
	virtual void arrowUp(Sctp* sctp) = 0;
	virtual void arrowDown(Sctp* sctp) = 0;
	virtual void arrowLeft(Sctp* sctp) = 0;
	virtual void arrowRight(Sctp* sctp) = 0;
	virtual void refreshLcd(Sctp* sctp, command_t command) = 0;
	virtual bool batteryIndicator() = 0;
	virtual void exit(Sctp* sctp) = 0;
	virtual int id(Sctp* sctp) = 0;

	virtual ~SctpState() {}
};