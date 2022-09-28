// built-in libraries
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include "freertos/queue.h"

// external libraries
#include <button.h>
#include <pcf8574.h>

// internal libraries
#include "sctp.h"
#include "sctp_buttons.h"
#include "sctp_lcd.h"

static const char TAG[] = "main";
i2c_dev_t pcf;
uint8_t pcf_val = 0xFF;
uint8_t button;

Sctp sctp;
QueueHandle_t button_events;

void buttons_task() {
    for (;;) {
        // button_event_t ev;
        // if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {
        //     if ((ev.pin == BUTTON_PIN_DOWN) && ((ev.event == BUTTON_DOWN) || ev.event == BUTTON_HELD)) {
        //         ESP_LOGI(TAG, "button 1 pressed");
        //         sctp.arrowDown();

        //     }
        //     if ((ev.pin == BUTTON_PIN_OK) && ((ev.event == BUTTON_DOWN) || ev.event == BUTTON_HELD)) {
        //         ESP_LOGI(TAG, "button 2 pressed");
        //         sctp.okay();
        //     }
        //     ESP_LOGI(TAG, "state=%d", sctp.getCurrentStateId());
        // }
        pcf8574_port_read(&pcf, &pcf_val);
        if ((pcf_val&1)==0) {
            ESP_LOGI(TAG, "button DOWN pressed");
            sctp.arrowDown();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>1)&1)==0) {
            ESP_LOGI(TAG, "button OK pressed");
            sctp.okay();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>2)&1)==0) {
            ESP_LOGI(TAG, "button UP pressed");
            sctp.arrowUp();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>3)&1)==0) {
            ESP_LOGI(TAG, "button LEFT pressed");
            sctp.arrowLeft();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        if (((pcf_val>>4)&1)==0) {
            ESP_LOGI(TAG, "button RIGHT pressed");
            sctp.arrowRight();
            vTaskDelay(200/portTICK_PERIOD_MS);
        }
        // ESP_LOGI(TAG, "state=%d", sctp.getCurrentStateId());
        vTaskDelay(400/portTICK_PERIOD_MS);
    }
}

extern "C" {

void app_main(void);

}


void lcdTask(void*) {
    for(;;) {
    }
}


void app_main() {
    ESP_LOGI(TAG, "HELLO WORLD :)");

    i2cdev_init();
    pcf8574_init(&pcf, &pcf_val);
    
    buttons_task();
    //DISPLAY TEST
    // float x[] = {400,400,401,402,402,403,404,404,405,406,407,407,408,409,409,410,411,411,412,413,413,414,415,416,416,417,418,418,419,420,420,421,422,422,423,424,425,425,426,427,427,428,429,429,430,431,431,432,433,433,434,435,436,436,437,438,438,439,440,440,441,442,442,443,444,445,445,446,447,447,448,449,449,450,451,451,452,453,454,454,455,456,456,457,458,458,459,460,460,461,462,462,463,464,465,465,466,467,467,468,469,470,470,471,472,473,473,474,475,476,476,477,478,479,479,480,481,482,482,483,484,485,485,486,487,488,488,489,490,491,491,492,493,494,494,495,496,497,497,498,499,500,500,501,502,503,503,504,505,506,506,507,508,509,509,510,511,512,512,513,514,515,515,516,517,518,518,519,520,521,521,522,523,524,524,525,526,527,527,528,529,530,530,531,532,533,533,534,535,535,536,537,537,538,539,539,540,541,542,542,543,544,544,545,546,546,547,548,548,549,550,550,551,552,553,553,554,555,555,556,557,557,558,559,559,560,561,561,562,563,563,564,565,566,566,567,568,568,569,570,570,571,572,572,573,574,574,575,576,577,577,578,579,579,580,581,581,582,583,583,584,585,585,586,587,588,588,589,590,590,591,592,592,593,594,594,595,596,596,597,598,599,599,600,601,601,602,603,603,604,605,605,606,607,607,608,609,609,610,611,612,612,613,614,614,615,616,616,617,618,618,619,620,620,621,622,623,623,624,625,625,626,627,627,628,629,629,630,631,631,632,633,634,634,635,636,636,637,638,638,639,640,640,641,642,642,643,644,645,645,646,647,647,648,649,649,650,651,651,652,653,653,654,655,655,656,657,658,658,659,660,660,661,662,662,663,664,664,665,666,666,667,668,669,669,670,671,671,672,673,673,674,675,675,676,677,677,678,679,680,680,681,682,682,683,684,684,685,686,686,687,688,688,689,690,691,691,692,693,693,694,695,695,696,697,697,698,699,699,700};
    // float y[] = {0.1,0.08,0.04,0.04,0.02,0.01,0.07,0.07,0.06,0.04,0.03,0.04,0.06,0.06,0.04,0.06,0.05,0.09,0.09,0.08,0.09,0.09,0.13,0.08,0.12,0.15,0.14,0.16,0.18,0.17,0.16,0.17,0.17,0.17,0.21,0.23,0.2,0.2,0.21,0.22,0.25,0.23,0.27,0.25,0.24,0.27,0.28,0.28,0.29,0.28,0.31,0.3,0.3,0.29,0.28,0.3,0.29,0.29,0.29,0.3,0.31,0.3,0.32,0.35,0.35,0.36,0.33,0.33,0.35,0.31,0.31,0.31,0.3,0.33,0.33,0.35,0.34,0.34,0.35,0.33,0.32,0.3,0.29,0.28,0.28,0.27,0.3,0.29,0.3,0.29,0.29,0.29,0.28,0.26,0.25,0.25,0.25,0.25,0.27,0.27,0.3,0.31,0.32,0.31,0.3,0.28,0.26,0.24,0.24,0.25,0.28,0.32,0.34,0.32,0.29,0.28,0.27,0.27,0.28,0.31,0.34,0.36,0.35,0.33,0.31,0.28,0.28,0.26,0.26,0.27,0.28,0.29,0.31,0.3,0.28,0.27,0.27,0.27,0.26,0.25,0.24,0.25,0.26,0.28,0.28,0.28,0.25,0.23,0.21,0.2,0.24,0.27,0.32,0.31,0.29,0.25,0.23,0.22,0.25,0.28,0.31,0.35,0.35,0.31,0.28,0.23,0.25,0.26,0.31,0.36,0.34,0.33,0.31,0.29,0.28,0.28,0.28,0.31,0.34,0.34,0.32,0.31,0.29,0.28,0.26,0.28,0.3,0.34,0.36,0.34,0.32,0.29,0.29,0.31,0.33,0.35,0.36,0.37,0.36,0.36,0.35,0.34,0.34,0.35,0.37,0.37,0.39,0.4,0.38,0.36,0.33,0.32,0.33,0.39,0.43,0.4,0.39,0.35,0.35,0.36,0.37,0.39,0.4,0.41,0.41,0.39,0.36,0.35,0.37,0.4,0.43,0.43,0.42,0.4,0.38,0.38,0.39,0.4,0.42,0.44,0.43,0.42,0.39,0.39,0.41,0.44,0.49,0.48,0.48,0.46,0.46,0.45,0.45,0.47,0.5,0.53,0.53,0.52,0.5,0.47,0.5,0.54,0.57,0.58,0.58,0.54,0.54,0.55,0.57,0.59,0.62,0.63,0.64,0.63,0.63,0.63,0.65,0.67,0.7,0.72,0.73,0.74,0.73,0.7,0.69,0.71,0.73,0.78,0.8,0.77,0.74,0.72,0.75,0.77,0.8,0.81,0.79,0.78,0.76,0.76,0.76,0.8,0.81,0.84,0.83,0.8,0.78,0.77,0.8,0.84,0.86,0.86,0.83,0.81,0.8,0.82,0.83,0.85,0.87,0.86,0.85,0.82,0.78,0.8,0.83,0.86,0.88,0.86,0.81,0.8,0.81,0.84,0.88,0.93,0.93,0.93,0.91,0.88,0.87,0.89,0.92,0.97,0.96,0.93,0.86,0.83,0.86,0.93,0.93,0.94,0.9,0.83,0.79,0.79,0.8,0.8,0.8,0.78,0.75,0.71,0.69,0.67,0.69,0.69,0.67,0.68,0.67,0.64,0.63,0.62,0.58,0.57,0.53,0.51,0.52,0.5,0.44,0.44,0.41,0.42,0.4,0.4,0.4,0.37,0.36,0.34,0.34,0.3,0.28,0.26,0.28,0.27,0.24,0.24,0.24,0.24,0.23,0.22,0.22,0.21,0.18,0.18,0.17,0.17,0.18,0.16,0.17,0.16,0.14,0.12,0.14,0.16,0.16,0.15,0.14,0.13,0.13,0.12,0.12,0.12,0.12,0.12,0.11,0.11,0.11,0.12,0.11,0.12,0.13,0.13};
    // sctp_lcd_spec_result(0, x, y, 430);

    //sctp_buttons_init(&button_events);

    //xTaskCreate(&buttons_task, "button_handler", CONFIG_ESP32_BUTTON_TASK_STACK_SIZE, NULL, 10, NULL);
    //xTaskCreatePinnedToCore( lcdTask, "lcdTask", 8192, NULL, 1, NULL, 1 );

}