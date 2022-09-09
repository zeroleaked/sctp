#pragma once

#include <sctp.h>

// #include <stdint.h>

class Sctp;
// Forward declaration to resolve circular dependency/include
class SctpState
{
public:
	virtual void enter(Sctp* sctp) = 0;
	virtual void toggle(Sctp* sctp) = 0;
	virtual void exit(Sctp* sctp) = 0;
	virtual int id(Sctp* sctp) = 0;

	virtual ~SctpState() {}
};