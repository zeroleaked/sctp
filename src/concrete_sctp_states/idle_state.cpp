#include "sctp_lcd.h"
#include "idle_state.h"
#include "menu_state.h"

void Idle::enter(Sctp* sctp)
{
	sctp_lcd_start();
	// Off -> Low
}

void Idle::toggle(Sctp* sctp)
{
	// Off -> Low
	sctp->setState(Menu::getInstance());
}

SctpState& Idle::getInstance()
{
	static Idle singleton;
	return singleton;
}