#pragma once

#include "sctp_state.h"
#include "sctp.h"

class ConcSample : public SctpState
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
	void exit(Sctp* sctp);
	int id(Sctp* sctp) { return 13; };
	static SctpState& getInstance();

	uint8_t substate;
	uint8_t cursor;
	uint16_t * check_result;
private:
	ConcSample() {}
	ConcSample(const ConcSample& other);
	ConcSample& operator=(const ConcSample& other);

	float absorbance;

	TaskHandle_t taskHandle;
	void * taskParam;
	QueueHandle_t report_queue;
};