#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>

#include "sctp_state.h"
#include "sctp_common_types.h"

class SctpState; // forward declaration

class Sctp
{
public:
	Sctp();
	// Same as before
	inline SctpState* getCurrentState() const { return currentState; }
	// In here, we'll delegate the state transition to the currentState
	void toggle();

	// button OK
	void okay();

	// button arrow up
	void arrowUp();

	// button arrow down
	void arrowDown();

	// button arrow left
	void arrowLeft();

	// button arrow right
	void arrowRight();

	// This will get called by the current state
	void setState(SctpState& newState);


    int getCurrentStateId();

	static void sampleSpectrumBlankWrapper(void * pvParameter);
	static void sampleSpectrumSampleWrapper(void * pvParameter);

	static void refreshLcdWrapper(void * pvParameter);

    TaskHandle_t task_spectrum_blank;
	TaskHandle_t task_spectrum_sample;

	QueueHandle_t lcd_refresh_queue;

	calibration_t calibration;

	blank_take_t blank_take;
	sample_take_t sample_take;

	uint16_t wavelength;
	curve_t * curve; // max 6!
	uint8_t curve_length = 0;
	
	history_t * history_list;
	uint8_t history_list_length;
	uint8_t history_index;
	history_t history;



private:
	SctpState* currentState;

	void sampleSpectrumBlank();
	void sampleSpectrumSample();
	void refreshLcd();
};