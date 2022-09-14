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
		absorbance[i] = transmission;
	}
	ESP_LOGI(TAG, "%f,%f,%f,%f,%f,%f,%f,%f",
		absorbance[0], absorbance[1], absorbance[2], absorbance[3], absorbance[4], absorbance[5], absorbance[6], absorbance[7]);
	ESP_LOGI(TAG, "%f,%f,%f,%f,%f,%f,%f,%f",
		sample_buffer[0], sample_buffer[1], sample_buffer[2], sample_buffer[3], sample_buffer[4], sample_buffer[5], sample_buffer[6], sample_buffer[7]);
	ESP_LOGI(TAG, "%f,%f,%f,%f,%f,%f,%f,%f",
		sample_buffer[calibration.length-8], sample_buffer[calibration.length-7], sample_buffer[calibration.length-6], sample_buffer[calibration.length-5],
		sample_buffer[calibration.length-4], sample_buffer[calibration.length-3], sample_buffer[calibration.length-2], sample_buffer[calibration.length-1]);

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