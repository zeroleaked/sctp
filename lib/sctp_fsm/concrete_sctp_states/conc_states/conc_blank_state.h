#pragma once

#include "sctp_state.h"
#include "sctp.h"

class ConcBlank : public SctpState
{
public:
	void enter(Sctp* sctp) {};
	void okay(Sctp* sctp) {};
	void arrowUp(Sctp* sctp) {};
	void arrowDown(Sctp* sctp) {};
	void arrowLeft(Sctp* sctp) {};
	void arrowRight(Sctp* sctp) {};
	void refreshLcd(Sctp* sctp, command_t command) {};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 4; }
	static SctpState& getInstance();

	uint8_t substate;

private:
	ConcBlank() {}
	ConcBlank(const ConcBlank& other);
	ConcBlank& operator=(const ConcBlank& other);

	uint8_t cursor;

	QueueHandle_t report_queue;
	TaskHandle_t taskHandle;
	void * taskParam; // necessary for freeing...
};