#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include <button.h>

#include "sctp_buttons.h"

static const char TAG[] = "sctp_buttons";

void sctp_buttons_init(QueueHandle_t * button_events) {
    * button_events = pulled_button_init(PIN_BIT(BUTTON_PIN_DOWN) | PIN_BIT(BUTTON_PIN_OK), GPIO_PULLUP_ONLY);
}

void lib_test(void) {
    ESP_LOGI(TAG, "lib test");
}