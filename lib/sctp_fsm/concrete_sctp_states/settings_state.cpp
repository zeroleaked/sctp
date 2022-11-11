#include <esp_log.h>
#include "settings_state.h"
#include "menu_state.h"
#include "sctp_lcd.h"

#define CURSOR_LOAD 0
#define CURSOR_BACK 1

static const char TAG[] = "settings_state";

void Settings::enter(Sctp *sctp)
{
	ESP_LOGI(TAG, "entered settings");
	sctp_lcd_clear();
	this->cursor = 0;
	sctp_lcd_menu(this->cursor);
}

void Settings::okay(Sctp *sctp)
{
	// Low -> Medium
	switch (this->cursor)
	{
		case CURSOR_LOAD:
		{
			break;
		}
		case CURSOR_BACK:
		{
			sctp->setState(Menu::getInstance());
			break;
		}
	}
}

void Settings::arrowUp(Sctp *sctp)
{
	sctp_lcd_menu_clear(this->cursor);
	if (this->cursor == 0)
		this->cursor = 1;
	else
		this->cursor--;
	sctp_lcd_menu(this->cursor);
}

void Settings::arrowDown(Sctp *sctp)
{
	sctp_lcd_menu_clear(this->cursor);
	if (this->cursor == 1)
		this->cursor = 0;
	else
		this->cursor++;
	sctp_lcd_menu(this->cursor);
}

SctpState& Settings::getInstance()
{
	static Settings singleton;
	return singleton;
}