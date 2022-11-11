#include <esp_log.h>
#include "settings_state.h"
#include "menu_state.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_CHECK 0
#define CURSOR_LOAD 1
#define CURSOR_BACK 2

static const char TAG[] = "settings_state";

void Settings::enter(Sctp *sctp)
{
	ESP_LOGI(TAG, "entered settings");
	sctp_lcd_clear();
	this->cursor = 0;
	sctp_lcd_settings(this->cursor);
}

void Settings::okay(Sctp *sctp)
{
	// Low -> Medium
	switch (this->cursor)
	{
		case CURSOR_CHECK:
		{
			sctp_lcd_settings_check(sctp->calibration);
			break;
		}
		case CURSOR_LOAD:
		{
			sctp_flash_load_calibration(&sctp->calibration);
			sctp_flash_nvs_save_calibration(sctp->calibration);
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
	sctp_lcd_settings_clear(this->cursor);
	if (this->cursor == 0)
		this->cursor = 2;
	else
		this->cursor--;
	sctp_lcd_settings(this->cursor);
}

void Settings::arrowDown(Sctp *sctp)
{
	sctp_lcd_settings_clear(this->cursor);
	if (this->cursor == 2)
		this->cursor = 0;
	else
		this->cursor++;
	sctp_lcd_settings(this->cursor);
}

SctpState& Settings::getInstance()
{
	static Settings singleton;
	return singleton;
}