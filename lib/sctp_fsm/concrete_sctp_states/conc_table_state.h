#pragma once

#include "sctp_state.h"
#include "sctp.h"

// standards library (storage)


class ConcTable : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp) {};
	void arrowUp(Sctp* sctp) {};
	void arrowDown(Sctp* sctp) {};
	void arrowLeft(Sctp* sctp) {};
	void arrowRight(Sctp* sctp) {};
	void refreshLcd(Sctp* sctp, command_t command) {};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 10; }
	static SctpState& getInstance();

	uint8_t substate;

private:
	ConcTable() {}
	ConcTable(const ConcTable& other);
	ConcTable& operator=(const ConcTable& other);

	uint8_t cursor;
};