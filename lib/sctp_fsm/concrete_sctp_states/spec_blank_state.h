#pragma once

#include "sctp_state.h"
#include "sctp.h"

// standards library (storage)

class SpecBlank : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp) { arrowLeft(sctp); };
	void arrowDown(Sctp* sctp) { arrowLeft(sctp); };
	void arrowLeft(Sctp* sctp);
	void arrowRight(Sctp* sctp) { arrowLeft(sctp); };
	void refreshLcd(Sctp* sctp, command_t command);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 3; }
	static SctpState& getInstance();

	uint8_t substate;
	uint8_t cursor;
private:
	SpecBlank() {}
	SpecBlank(const SpecBlank& other);
	SpecBlank& operator=(const SpecBlank& other);
};