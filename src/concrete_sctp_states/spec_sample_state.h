#pragma once

#include "sctp_state.h"
#include "sctp.h"

// standards library (storage)

class SpecSample : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp) { arrowLeft(sctp); };
	void arrowDown(Sctp* sctp) { arrowLeft(sctp); };
	void arrowLeft(Sctp* sctp);
	void arrowRight(Sctp* sctp) { arrowLeft(sctp); };
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 3; }
	static SctpState& getInstance();

private:
	SpecSample() {}
	SpecSample(const SpecSample& other);
	SpecSample& operator=(const SpecSample& other);

	uint8_t substate;
	uint8_t cursor;
};