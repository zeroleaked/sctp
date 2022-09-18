#pragma once

#include "sctp_state.h"
#include "sctp.h"

class ConcRegress : public SctpState
{
public:
	void enter(Sctp* sctp) {};
	void okay(Sctp* sctp) {};
	void arrowUp(Sctp* sctp) { arrowLeft(sctp); };
	void arrowDown(Sctp* sctp) { arrowLeft(sctp); };
	void arrowLeft(Sctp* sctp) {};
	void arrowRight(Sctp* sctp) { arrowLeft(sctp); };
	void refreshLcd(Sctp* sctp, command_t command) {};
	void exit(Sctp* sctp) {};
	int id(Sctp* sctp) { return 14; };
	static SctpState& getInstance();

	uint8_t substate;
	uint8_t cursor;
private:
	ConcRegress() {}
	ConcRegress(const ConcRegress& other);
	ConcRegress& operator=(const ConcRegress& other);

	TaskHandle_t taskHandle;
	void * taskParam;
	QueueHandle_t report_queue;
};