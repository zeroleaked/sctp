#pragma once

#include "sctp_state.h"
#include "sctp.h"

class ConcRegress : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp) { arrowDown(sctp); };
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp) { arrowDown(sctp); };
	void arrowRight(Sctp* sctp) { arrowDown(sctp); };
	void refreshLcd(Sctp* sctp, command_t command) {};
	void exit(Sctp* sctp) {};
	int id(Sctp* sctp) { return 14; };
	static SctpState& getInstance();

private:
	ConcRegress() {}
	ConcRegress(const ConcRegress& other);
	ConcRegress& operator=(const ConcRegress& other);
	
	uint8_t cursor;
	uint8_t substate;

	uint8_t standards_length;
	bool interpolate;
};