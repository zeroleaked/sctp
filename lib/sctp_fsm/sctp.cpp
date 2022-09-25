#include <esp_log.h>
#include <math.h>

#include "sctp.h"
#include "concrete_sctp_states/concrete_sctp_states.h"
#include "sctp_sensor.h"
#include "sctp_flash.h"

static const char TAG[] = "sctp";

// Constructor
Sctp::Sctp()
{
	// todo load calibration
	calibration.row = 477;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;

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


void Sctp::refreshLcd()
{
	ESP_LOGI(TAG, "refresh LCD task started");

	command_t command;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	QueueHandle_t queue = lcd_refresh_queue; // copy handle as local variable, somehow task loop don't like member variables
	ESP_LOGI(TAG, "refreshLcd: queue=0x%08x", (unsigned) queue);
	for (;;) {
		vTaskDelayUntil( &xLastWakeTime, 300 / portTICK_RATE_MS );
		if (xQueueReceive(queue, &command, 0) == pdTRUE) {
			ESP_LOGI(TAG, "refreshLcd(), delegating");
			currentState->refreshLcd(this, command);
		}
		else {
			command = COMMAND_NONE;
			currentState->refreshLcd(this, command);
		}
	}
}

void Sctp::refreshLcdWrapper(void * _this)
{
	((Sctp *) _this)->refreshLcd();
}