#pragma once

#include "sctp_state.h"
#include "sctp.h"

class ConcWavelength : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp);
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp);
	void arrowRight(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 4; }
	static SctpState& getInstance();

private:
	ConcWavelength() {}
	ConcWavelength(const ConcWavelength& other);
	ConcWavelength& operator=(const ConcWavelength& other);

	uint8_t substate;
	uint8_t cursor;
	uint16_t wavelength;
};