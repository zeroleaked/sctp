#pragma once

#include "sctp_state.h"
#include "sctp.h"

class HistoryList : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp);
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp);
	void arrowRight(Sctp* sctp);
	void refreshLcd(Sctp* sctp, command_t command) {};
	bool batteryIndicator() {return true;};
	void exit(Sctp* sctp);
	int id(Sctp* sctp) { return 5; }
	static SctpState& getInstance();

	void updateLcd(Sctp * sctp);

private:
	HistoryList() {}
	HistoryList(const HistoryList& other);
	HistoryList& operator=(const HistoryList& other);

    uint8_t cursor;
    uint8_t offset;
	uint8_t substate;

	uint8_t file_count;
	char (*filenames)[25];
	curve_t * history_curve;
	conc_regression_t * regress_line;
	bool interpolate = false;
	float * history_wavelength;
	float * history_absorbance;
	uint16_t * spectrum_length;
};