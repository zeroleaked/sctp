#pragma once

#define BUTTON_PIN_DOWN 0  //strobe
#define BUTTON_PIN_OK 1    //trig
#define BUTTON_THRESHOLD 20

typedef enum {waiting, detected, unreleased, update} button_state;

void pcf8574_init(i2c_dev_t *pcf, uint8_t *pcf_val);

void sctp_buttons_init(QueueHandle_t * button_events);

void lib_test(void);

void button_debounce(   int input,
                        button_state *state,
                        volatile int *output,
                        int *count);