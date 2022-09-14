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
	void refreshLcd(Sctp* sctp) {};
	void exit(Sctp* sctp);
	int id(Sctp* sctp) { return 5; }
	static SctpState& getInstance();

	void updateLcd(Sctp * sctp);

private:
	HistoryList() {}
	HistoryList(const HistoryList& other);
	HistoryList& operator=(const HistoryList& other);

    uint8_t cursor;
    uint8_t page;
	uint8_t page_length;
	bool next_page;
};