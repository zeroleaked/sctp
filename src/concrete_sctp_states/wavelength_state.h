#pragma once

#include "sctp_state.h"
#include "sctp.h"

class Wavelength : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp);
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp);
	void arrowRight(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 3; }
	static SctpState& getInstance();

private:
	Wavelength() {}
	Wavelength(const Wavelength& other);
	Wavelength& operator=(const Wavelength& other);

	uint8_t button_set;
	uint8_t cursor;
	uint16_t wavelength;
};