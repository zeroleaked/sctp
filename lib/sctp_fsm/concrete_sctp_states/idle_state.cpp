#include "sctp_lcd.h"
#include "idle_state.h"
#include "menu_state.h"

void Idle::enter(Sctp* sctp)
{
	sctp_lcd_start();
    xTaskCreatePinnedToCore(sctp->refreshLcdWrapper, "LCD refresh", 2048, sctp, 3, &sctp->task_refresh_lcd, 0);
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