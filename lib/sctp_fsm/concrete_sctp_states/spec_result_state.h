#pragma once

#include "sctp_state.h"
#include "sctp.h"

class SpecResult : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp);
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp) { arrowUp(sctp); };
	void arrowRight(Sctp* sctp) { arrowDown(sctp); };
	void refreshLcd(Sctp* sctp) {};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 2; }
	static SctpState& getInstance();

private:
	SpecResult() {}
	SpecResult(const SpecResult& other);
	SpecResult& operator=(const SpecResult& other);

    uint8_t cursor;
    uint8_t substate;
};