#include <esp_log.h>

#include "sctp.h"

char TAG[] = "main";

Sctp sctp;

void fsm() {
    SctpState* sctp_state = sctp.getCurrentState();
    int current_id = sctp.getCurrentStateId();
    ESP_LOGI(TAG, "state=%d", current_id);
}

extern "C" {

void app_main() {
    ESP_LOGI(TAG, "HELLO WORLD :)");
    fsm();
}

}