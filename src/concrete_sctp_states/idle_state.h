#pragma once

#include "sctp_state.h"
#include "sctp.h"

class Idle : public SctpState
{
public:
	void enter(Sctp* sctp);
	void arrowUp(Sctp* sctp) { toggle(sctp); }
	void arrowDown(Sctp* sctp) { toggle(sctp); }
	void arrowLeft(Sctp* sctp) { toggle(sctp); }
	void arrowRight(Sctp* sctp) { toggle(sctp); }
	void okay(Sctp* sctp) { toggle(sctp); }
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 1; };
	static SctpState& getInstance();

	void toggle(Sctp* sctp);

private:
	Idle() {}
	Idle(const Idle& other);
	Idle& operator=(const Idle& other);
};
