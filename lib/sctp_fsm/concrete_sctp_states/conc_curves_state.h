#pragma once

#include "sctp_state.h"
#include "sctp.h"

// standards library (storage)


class ConcCurves : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp) {};
	void arrowDown(Sctp* sctp) {};
	void arrowLeft(Sctp* sctp) {};
	void arrowRight(Sctp* sctp) {};
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 4; }
	static SctpState& getInstance();

private:
	ConcCurves() {}
	ConcCurves(const ConcCurves& other);
	ConcCurves& operator=(const ConcCurves& other);

	uint8_t cursor;

	curve_t * curves;
	uint8_t curve_length;
};