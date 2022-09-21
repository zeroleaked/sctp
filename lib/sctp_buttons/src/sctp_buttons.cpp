#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <string.h>

#include <button.h>
#include <pcf8574.h>

#include "sctp_buttons.h"

static const char TAG[] = "sctp_buttons";

void pcf8574_init(i2c_dev_t *pcf, uint8_t *pcf_val) {
    // PCF TEST
    // Zero device descriptor
    memset(pcf, 0, sizeof(i2c_dev_t));

    // Init i2c device descriptor
    pcf8574_init_desc(pcf, I2C_ADDR, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL);
}

void sctp_buttons_init(QueueHandle_t * button_events) {
    //* button_events = pulled_button_init(pcf8574_port_read(&pcf, &pcf_val), GPIO_FLOATING);
    //ESP_LOGI(TAG, "pcf_val=%d", pcf8574_port_read(&pcf, &pcf_val));
}

void lib_test(void) {
    ESP_LOGI(TAG, "lib test");
}

void button_debounce(int input, button_state *state, volatile int *output, int *count)
{ 
  switch(*state){
    case waiting:{
      if(input==1){
        *state = detected;
        *output = 1;
        ++(*count);
      }
      else if(input==0){
        *state = *state;
        *output = 0;
        *count = 0;
      }
      break;
    }
    case detected:{
      if(*count >= BUTTON_THRESHOLD){
        if(input==1){
          *state = detected;
          *output = 1;
          ++(*count);
        }
        else if(input==0){
          *state = waiting;
          *output = 0;
          *count = 0;
        }
      }
      else{
        *state = *state;
        *output = 0;
        ++(*count);
      }
      break;
    }
    case unreleased:{
      if(input==1){
        *state = *state;
        *output = 0;
        *count = 0;
      }
      else if(input==0){
        *state = update;
        *output = 0;
        ++(*count);
      }
      break;
    }
    case update:{
      if(*count >= BUTTON_THRESHOLD){
        if(input==1){
          *state = detected;
          *output = 0;
          *count = 0;
        }
        else if(input==0){
          *state = waiting;
          *output = 0;
          *count = 0;
        }
      }
      else{
        *state = *state;
        *output = 0;
        ++(*count);
      }
      break;
    }
  }
}