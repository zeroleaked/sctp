#pragma once

#include "sctp.h"
#include "sctp_state.h"
#include "sctp_common_types.h"

class SpecSave : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp) { arrowDown(sctp); };
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp) { arrowDown(sctp); };
	void arrowRight(Sctp* sctp) { arrowDown(sctp); };
	void refreshLcd(Sctp* sctp, command_t command);
	bool batteryIndicator() {return true;};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 9; }
	static SctpState& getInstance();

    uint8_t substate;

private:
	SpecSave() {}
	SpecSave(const SpecSave& other);
	SpecSave& operator=(const SpecSave& other);

    uint8_t cursor;

	TaskHandle_t taskHandle;
	void * taskParam;
	QueueHandle_t report_queue;
};