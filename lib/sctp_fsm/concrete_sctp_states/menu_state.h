#pragma once

#include "sctp_state.h"
#include "sctp.h"

class Menu : public SctpState
{
public:
	void enter(Sctp* sctp);
	void okay(Sctp* sctp);
	void arrowUp(Sctp* sctp);
	void arrowDown(Sctp* sctp);
	void arrowLeft(Sctp* sctp) { arrowUp(sctp); };
	void arrowRight(Sctp* sctp) { arrowDown(sctp); };
	void refreshLcd(Sctp* sctp, command_t command);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp) { return 2; }
	static SctpState& getInstance();

private:
	Menu() {}
	Menu(const Menu& other);
	Menu& operator=(const Menu& other);

    uint8_t cursor;
};