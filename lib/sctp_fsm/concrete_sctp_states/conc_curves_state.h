#pragma once

#include "sctp_state.h"
#include "sctp.h"

// standards library (storage)


class ConcCurves : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp);
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp) { arrowRight(sctp); };
	void arrowRight(Sctp* sctp);
	void refreshLcd(Sctp* sctp, command_t command);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 4; }
	static SctpState& getInstance();

	uint8_t substate;

private:
	ConcCurves() {}
	ConcCurves(const ConcCurves& other);
	ConcCurves& operator=(const ConcCurves& other);

	uint8_t cursor;

	char filenames[6][20];
	curve_t curves[6];
};