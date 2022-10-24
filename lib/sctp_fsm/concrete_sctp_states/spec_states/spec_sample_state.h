#pragma once

#include "sctp.h"
#include "sctp_state.h"

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
	void refreshLcd(Sctp* sctp, command_t command);
	bool batteryIndicator() {return false;};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 7; }
	static SctpState& getInstance();

	uint8_t substate;
	uint8_t cursor;
private:
	SpecSample() {}
	SpecSample(const SpecSample& other);
	SpecSample& operator=(const SpecSample& other);

	TaskHandle_t taskHandle;
	void * taskParam;
	QueueHandle_t report_queue;
};