#pragma once

#define BUTTON_PIN_DOWN 39
#define BUTTON_PIN_OK 40


void sctp_buttons_init(QueueHandle_t * button_events);

void lib_test(void);

