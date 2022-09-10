#include "sctp_lcd.h"
#include "concrete_sctp_states.h"

void LightOff::toggle(Sctp* sctp)
{
	// Off -> Low
	sctp->setState(LowIntensity::getInstance());
}

void LightOff::enter(Sctp* sctp)
{
	sctp_lcd_start();
	// Off -> Low
}

int LightOff::id(Sctp* sctp)
{
	return 1;
}

SctpState& LightOff::getInstance()
{
	static LightOff singleton;
	return singleton;
}

void LowIntensity::okay(Sctp* sctp)
{
	// Low -> Medium
	switch (sctp->cursor) {
		case 1: {
			sctp->setState(MediumIntensity::getInstance());
			break;
		}
	}
}

void LowIntensity::arrowDown(Sctp* sctp)
{
	sctp_lcd_menu_clear(sctp->cursor);
	sctp->cursor++;
	if (sctp->cursor >= 4) sctp->cursor = 0;
	sctp_lcd_menu(sctp->cursor);
}

void LowIntensity::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	sctp->cursor = 0;
	sctp_lcd_menu(sctp->cursor);
	// Off -> Low
}

int LowIntensity::id(Sctp* sctp)
{
	return 2;
}

SctpState& LowIntensity::getInstance()
{
	static LowIntensity singleton;
	return singleton;
}

void MediumIntensity::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	sctp->cursor = 550;
	sctp_lcd_conc_input_1(sctp->cursor);
	// Off -> Low
}

void MediumIntensity::arrowDown(Sctp* sctp)
{
	sctp_lcd_clear(); // laggy, change this to sctp_lcd_conc_input_1_clear
	sctp->cursor = sctp->cursor - 2;
	if (sctp->cursor < 400) sctp->cursor = 700;
	sctp_lcd_conc_input_1(sctp->cursor);
}

int MediumIntensity::id(Sctp* sctp)
{
	return 3;
}

SctpState& MediumIntensity::getInstance()
{
	static MediumIntensity singleton;
	return singleton;
}

int HighIntensity::id(Sctp* sctp)
{
	return 4;
}

SctpState& HighIntensity::getInstance()
{
	static HighIntensity singleton;
	return singleton;
}