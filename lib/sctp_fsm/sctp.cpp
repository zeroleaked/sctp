#include <esp_log.h>

#include "sctp.h"
#include "concrete_sctp_states/concrete_sctp_states.h"
#include "sctp_sensor.h"

static const char TAG[] = "sctp";

// Constructor
Sctp::Sctp()
{
	// todo load calibration
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;

	lcd_refresh_queue = xQueueCreate(5, sizeof(command_t));
	assert(lcd_refresh_queue != NULL);

    currentState = &Idle::getInstance();
	currentState->enter(this);
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

void Sctp::sampleSpectrumBlank() {
	// Sctp class is responsible for all memory allocation it uses
	blank_take.readout = (float *) malloc(sizeof(float) * calibration.length);
	assert(blank_take.readout != NULL);
	blank_take.exposure = 10;
	blank_take.gain = 1;
	ESP_ERROR_CHECK(sctp_sensor_spectrum_blank(calibration, blank_take));

	currentState->exit(this);  // do stuff before we change state
	currentState = &SpecSample::getInstance();  // change state
	currentState->enter(this); // do stuff after we change state
	
	command_t command = SPECTRUM_BLANK;
	assert(xQueueSend(lcd_refresh_queue, &command, 0) == pdTRUE);
	vTaskDelete( NULL );
}

void Sctp::sampleSpectrumBlankWrapper(void * _this)
{
	((Sctp *) _this)->sampleSpectrumBlank();
}

void Sctp::sampleSpectrumSample() {
	// Sctp class is responsible for all memory allocation it uses
	sample_take = (float *) malloc(sizeof(float) * calibration.length);
	assert(sample_take != NULL);
	ESP_ERROR_CHECK(sctp_sensor_spectrum_sample(calibration, blank_take, sample_take));

	currentState->exit(this);  // do stuff before we change state
	currentState = &SpecResult::getInstance();  // change state
	currentState->enter(this); // do stuff after we change state
	
	command_t command = SPECTRUM_SAMPLE;
	assert(xQueueSend(lcd_refresh_queue, &command, 0) == pdTRUE);
	vTaskDelete( NULL );
}

void Sctp::sampleSpectrumSampleWrapper(void * _this)
{
	((Sctp *) _this)->sampleSpectrumSample();
}


void Sctp::refreshLcd()
{
	ESP_LOGI(TAG, "refresh LCD task started");
	command_t command;
	for (;;) {
		if (xQueueReceive(lcd_refresh_queue, &command, 100 / portTICK_PERIOD_MS) == pdTRUE) {
			currentState->refreshLcd(this);
		}
	}
}

void Sctp::refreshLcdWrapper(void * _this)
{
	((Sctp *) _this)->refreshLcd();
}