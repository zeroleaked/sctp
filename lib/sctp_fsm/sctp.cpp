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
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;

	lcd_refresh_queue = xQueueCreate(1, sizeof(command_t));
	assert(lcd_refresh_queue != NULL);
	ESP_LOGI(TAG, "created queue");

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
	ESP_LOGI(TAG, "sampleSpectrumBlank() sended to queue");
	vTaskDelete( NULL );
}

void Sctp::sampleSpectrumBlankWrapper(void * _this)
{
	((Sctp *) _this)->sampleSpectrumBlank();
}

void Sctp::sampleSpectrumSample() {
	// Sctp class is responsible for all memory allocation it uses
	sample_take = (float *) malloc(sizeof(sample_take_t) * calibration.length);
	absorbance = (float *) malloc(sizeof(absorbance_t) * calibration.length);
	assert(sample_take != NULL);
	assert(absorbance != NULL);

	ESP_ERROR_CHECK(sctp_sensor_spectrum_sample(calibration, blank_take, sample_take));
	// castings
	float * blank_buffer = blank_take.readout;
	float * sample_buffer = sample_take;
	for (int i=0; i < calibration.length; i++) {
		float transmission = sample_buffer[i]/blank_buffer[i];
		absorbance[i] = -log10(transmission);
	}

	currentState->exit(this);  // do stuff before we change state
	currentState = &SpecResult::getInstance();  // change state
	currentState->enter(this); // do stuff after we change state
	
	command_t command = SPECTRUM_SAMPLE;
	assert(xQueueSend(lcd_refresh_queue, &command, 0) == pdTRUE);
	ESP_LOGI(TAG, "sampleSpectrumSample() sended to queue");
	vTaskDelete( NULL );
}

void Sctp::sampleSpectrumSampleWrapper(void * _this)
{
	((Sctp *) _this)->sampleSpectrumSample();
}

void Sctp::saveSpectrum() {
	assert(absorbance != NULL);

	ESP_ERROR_CHECK(sctp_flash_save_spectrum(absorbance, calibration.length));
    SpecSave * specSave = (SpecSave *) currentState;
	specSave->substate = 1;

	command_t command = SPECTRUM_SAVE;
	assert(xQueueSend(lcd_refresh_queue, &command, 0) == pdTRUE);
	ESP_LOGI(TAG, "saveSpectrum() sended to queue");
	vTaskDelete( NULL );
}

void Sctp::saveSpectrumWrapper(void * _this)
{
	((Sctp *) _this)->saveSpectrum();
}


void Sctp::refreshLcd()
{
	ESP_LOGI(TAG, "refresh LCD task started");
	command_t command;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	QueueHandle_t queue = lcd_refresh_queue; // copy handle as local variable, somehow task loop don't like member variables
	for (;;) {
		vTaskDelayUntil( &xLastWakeTime, 300 / portTICK_RATE_MS );
		if (xQueueReceive(queue, &command, 0) == pdTRUE) {
			ESP_LOGI(TAG, "refreshLcd(), delegating");
			currentState->refreshLcd(this);
		}
	}
}

void Sctp::refreshLcdWrapper(void * _this)
{
	((Sctp *) _this)->refreshLcd();
}