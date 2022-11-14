#include <esp_log.h>
#include <math.h>

#include <i2cdev.h>

#include "sctp.h"
#include "concrete_sctp_states/concrete_sctp_states.h"
#include "sctp_sensor.h"
#include "sctp_flash.h"
#include "sctp_battery.h"
#include "sctp_lcd.h"

static const char TAG[] = "sctp";

// Constructor
Sctp::Sctp()
{
    i2cdev_init();
	sctp_battery_init();

	sctp_flash_nvs_load_calibration(&calibration);
	ESP_LOGI(TAG, "nvs loading passed");
	// todo load calibration
	// calibration.row = 490;
	// calibration.gain = -0.7666855524;
	// calibration.bias = 1013.975014;
	// calibration.start = 409;
	// calibration.length = 393;

	lcd_refresh_queue = xQueueCreate(1, sizeof(command_t));
	assert(lcd_refresh_queue != NULL);

    currentState = &Idle::getInstance();
	currentState->enter(this);

	ESP_LOGI(TAG, "constructor done");
}

void Sctp::setState(SctpState& newState)
{
	currentState->exit(this);  // do stuff before we change state
	currentState = &newState;  // change state
	currentState->enter(this); // do stuff after we change state
}

void Sctp::okay()
{
	// Delegate the task of determining the next state to the current state!
	currentState->okay(this);
}

void Sctp::arrowUp()
{
	// Delegate the task of determining the next state to the current state!
	currentState->arrowUp(this);
}

void Sctp::arrowDown()
{
	// Delegate the task of determining the next state to the current state!
	currentState->arrowDown(this);
}

void Sctp::arrowLeft()
{
	// Delegate the task of determining the next state to the current state!
	currentState->arrowLeft(this);
}

void Sctp::arrowRight()
{
	// Delegate the task of determining the next state to the current state!
	currentState->arrowRight(this);
}


int Sctp::getCurrentStateId()
{
    return currentState->id(this);
}

// void Sctp::saveSpectrum() {
// 	assert(absorbance != NULL);

// 	ESP_ERROR_CHECK(sctp_flash_save_spectrum(absorbance, calibration.length));

// 	command_t command = SPECTRUM_SAVE;
// 	assert(xQueueSend(lcd_refresh_queue, &command, 0) == pdTRUE);
// 	ESP_LOGI(TAG, "saveSpectrum() sended to queue");
// 	vTaskDelete( NULL );
// }

// void Sctp::saveSpectrumWrapper(void * _this)
// {
// 	((Sctp *) _this)->saveSpectrum();
// }

void Menu::refreshLcd(Sctp* sctp, command_t command)
{
	if (command == COMMAND_BAT_UPDATE) {
	}
}


void Sctp::refreshLcd()
{
	ESP_LOGI(TAG, "refresh LCD task started");

	command_t command;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	QueueHandle_t queue = lcd_refresh_queue; // copy handle as local variable, somehow task loop don't like member variables
	uint8_t counter_battery = 0;
	for (;;) {
		vTaskDelayUntil( &xLastWakeTime, 500 / portTICK_RATE_MS );
		if (xQueueReceive(queue, &command, 0) == pdTRUE) {
			ESP_LOGI(TAG, "refreshLcd(), delegating");
			currentState->refreshLcd(this, command);
		}
		else if (counter_battery == 2) {
			counter_battery = 0;
			// command = COMMAND_BAT_UPDATE;
			// currentState->refreshLcd(this, command);

			if (currentState->batteryIndicator()) {
				sctp_battery_sample(&this->battery_percentage);
				// ESP_LOGI(TAG,"perc=%d", this->battery_percentage);
				sctp_lcd_battery(this->battery_percentage);
			}

		}
		command = COMMAND_NONE;
		currentState->refreshLcd(this, command);
		counter_battery++;
	}
}

void Sctp::refreshLcdWrapper(void * _this)
{
	((Sctp *) _this)->refreshLcd();
}