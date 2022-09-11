#pragma once

#include "sctp_state.h"
#include "sctp.h"

// standards library (storage)

class Standards : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp) {};
	void arrowUp(Sctp* sctp) {};
	void arrowDown(Sctp* sctp) {};
	void arrowLeft(Sctp* sctp) {};
	void arrowRight(Sctp* sctp) {};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 3; }
	static SctpState& getInstance();

private:
	Standards() {}
	Standards(const Standards& other);
	Standards& operator=(const Standards& other);

	uint8_t button_set;
	uint8_t cursor;
};