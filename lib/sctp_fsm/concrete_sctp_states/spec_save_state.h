#pragma once

#include "sctp_state.h"
#include "sctp.h"

class SpecSave : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp) { arrowDown(sctp); };
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp) { arrowDown(sctp); };
	void arrowRight(Sctp* sctp) { arrowDown(sctp); };
	void refreshLcd(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 9; }
	static SctpState& getInstance();

    uint8_t substate;

private:
	SpecSave() {}
	SpecSave(const SpecSave& other);
	SpecSave& operator=(const SpecSave& other);

    uint8_t cursor;
};