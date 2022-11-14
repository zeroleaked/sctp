#include <esp_log.h>

#include "sctp_lcd.h"
#include "idle_state.h"
#include "menu_state.h"
#include "sctp_sensor.h"

static const char TAG[] = "idle_state"; 

void Idle::enter(Sctp* sctp)
{
	ESP_LOGI(TAG, "Init LCD");
	sctp_lcd_start();
	ESP_LOGI(TAG, "Init sensor");
	sctp_sensor_init();

	ESP_LOGI(TAG, "Idle::enter: queue=0x%08x", (unsigned) sctp->lcd_refresh_queue);
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