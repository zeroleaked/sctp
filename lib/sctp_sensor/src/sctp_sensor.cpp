#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <esp_log.h>

#include "sctp_sensor.h"
#include "sctp_camera.h"
#include "camera_config.h"

static const char TAG[] = "sctp_sensor";

#define PIN_LAMP_SWITCH GPIO_NUM_16

void buffer_flush() {
    camera_fb_t * take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
}

#define LAMP_CHECK_BUFFER_SIZE 20

esp_err_t halogen_wait(uint16_t pixel) {
    camera_fb_t * take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 1000 / portTICK_PERIOD_MS;
    uint16_t buffer[LAMP_CHECK_BUFFER_SIZE] = {0};
    uint16_t last_check = 0;
    bool isbreak = false;
    for (int i=0; i<90; i++) {

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        buffer_flush();
        take = sctp_camera_fb_get();
        uint16_t readout = (take->buf[pixel * 2] << 8) | (take->buf[pixel*2 + 1]);
        sctp_camera_fb_return(take);

        for (int j=0; j<LAMP_CHECK_BUFFER_SIZE; j++) {
            if (i >= j) buffer[j] += readout;
        }
        // ESP_LOGI(TAG, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", i, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4],
        // buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15], buffer[16],
        // buffer[17], buffer[18], buffer[19]);

        if (i >= LAMP_CHECK_BUFFER_SIZE) {
            uint8_t cursor = (i+1)%20;
            if (buffer[cursor] <= last_check) {
                ESP_LOGI(TAG, "%d <= %d", buffer[cursor], last_check);
                isbreak = true;
                break;
            }
        }
        if (i >= LAMP_CHECK_BUFFER_SIZE -1) {
            uint8_t cursor = (i+1)%20;
            last_check = buffer[cursor];
            buffer[cursor] = 0;
            // ESP_LOGI(TAG, "%d:%d", i, last_check);
        }
    }
    if (isbreak) return ESP_OK;
    else {
        ESP_LOGE(TAG, "unsteady readout");
        return ESP_FAIL;
    }
}

// called on sctp startup
esp_err_t sctp_sensor_init() {
    // standby pin init
    gpio_config_t conf = {};
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = 1LL << camera_config.pin_stnby;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);

    // start standby
    gpio_set_level( (gpio_num_t) camera_config.pin_stnby, 1);

    // lamp pin init
    conf = {};
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = 1LL << PIN_LAMP_SWITCH;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);

    // turn off level
    gpio_set_level( PIN_LAMP_SWITCH, 0);

    return ESP_OK;
};

void standby_start() {
    gpio_set_level((gpio_num_t) camera_config.pin_stnby, 1 );
}

void standby_end() {
    gpio_set_level((gpio_num_t) camera_config.pin_stnby, 0 );
}

esp_err_t sctp_sensor_spectrum_blank(calibration_t * calibration, blank_take_t * blank_take) {
    standby_end();

    sctp_camera_init(&camera_config);
    sensor_t *camera_sensor = sctp_camera_sensor_get();
    camera_sensor->set_row_start(camera_sensor, calibration->row);
    ESP_LOGI(TAG, "row set to %d", calibration->row);
    int exposure = blank_take->exposure[0];
    ESP_LOGI(TAG, "setting to %d exposure", exposure);
    camera_sensor->set_shutter_width(camera_sensor, exposure);

    int setpoint = 575;
    int error = setpoint;
    float kp = 0.2;
    const int tolerance = 25;
    gpio_set_level( PIN_LAMP_SWITCH, 1);
    // vTaskDelay(30000 / portTICK_PERIOD_MS);
    halogen_wait(calibration->start);
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    return ESP_FAIL;
    ESP_LOGI(TAG, "lamp heating done");

    buffer_flush();

    camera_fb_t * take = sctp_camera_fb_get();
    assert(take != NULL);

    bool max_exposure = false;
    for (int i=0; i<calibration->length; i++) {
        ESP_LOGI(TAG, "blank->readout[%d]", i);

        uint16_t pixel = calibration->start + calibration->length-1 - i;

        uint16_t readout = (take->buf[pixel * 2] << 8) | (take->buf[pixel*2 + 1]);
        error = setpoint - readout;     

        while ((error > tolerance) || (error < -tolerance) || max_exposure) {
            // ESP_LOGI(TAG, "pixel %d retake", pixel);
            sctp_camera_fb_return(take);
            exposure = exposure + kp * error;
            if (exposure <= 0) {
                exposure = 1;
            }
            else if (exposure > 3072) {
                max_exposure = true; // make this the last tale
                exposure = 3072;
            }

            ESP_LOGI(TAG, "setting to %d exposure", exposure);

            camera_sensor->set_shutter_width(camera_sensor, exposure);
            // flush
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);

            take = sctp_camera_fb_get();

            readout = (take->buf[pixel * 2] << 8) | (take->buf[pixel*2 + 1]);
            error = setpoint - readout;     
        }
        

        blank_take->exposure[i] = exposure;
        blank_take->readout[i] = readout;
    }
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    sctp_camera_fb_return(take);

    sctp_camera_deinit();

    for (int i=0; i<calibration->length; i++) {
        ESP_LOGI(TAG, "exposure[%d] = %d", i, blank_take->exposure[i]);
    }
    blank_take->gain = 1;
    
    standby_start();

    return ESP_OK;
};

esp_err_t sctp_sensor_spectrum_sample(calibration_t * calibration, blank_take_t * blank_take, float * sample_take) {
    // standby_end();

    // sctp_camera_init(&camera_config);
    // sensor_t *camera_sensor = sctp_camera_sensor_get();
    // camera_sensor->set_row_start(camera_sensor, calibration->row);
    // camera_sensor->set_shutter_width(camera_sensor, blank_take->exposure);

    // // flush
    // camera_fb_t * take = sctp_camera_fb_get();
    // sctp_camera_fb_return(take);
    // take = sctp_camera_fb_get();
    // sctp_camera_fb_return(take);
    // take = sctp_camera_fb_get();
    // sctp_camera_fb_return(take);
    // take = sctp_camera_fb_get();
    // sctp_camera_fb_return(take);

    // gpio_set_level( PIN_LAMP_SWITCH, 1);
    // take = sctp_camera_fb_get();
    // gpio_set_level( PIN_LAMP_SWITCH, 0);
    // for (int i=0; i < calibration->length; i++) {
    //     uint16_t readout = take->buf[(calibration->start+i) * 2] << 8 | take->buf[(calibration->start+i) * 2 + 1 ];
    //     sample_take[calibration->length - 1 - i] = readout;
    // }
    // sctp_camera_fb_return(take);

    // sctp_camera_deinit();

    // standby_start();

    return ESP_OK;
};

esp_err_t sctp_sensor_concentration_blank(calibration_t * calibration, uint16_t wavelength, blank_take_t * blank_take) {
    standby_end();

    sctp_camera_init(&camera_config);
    sensor_t *camera_sensor = sctp_camera_sensor_get();
    camera_sensor->set_row_start(camera_sensor, calibration->row);
    ESP_LOGI(TAG, "row set to %d", calibration->row);

    int exposure = *blank_take->exposure;
    int setpoint = 650;
    int error = setpoint;
    float kp = 1;
    int tolerance = 50;
    int iter = 0;

    // wl = px * gain + bias
    // px = (wl - bias) / gain
    uint16_t px = round((float) ((wavelength - calibration->bias) / calibration->gain));
    ESP_LOGI(TAG, "wl=%d, px=%d", wavelength, px);

    gpio_set_level( PIN_LAMP_SWITCH, 1);
    while ((error > tolerance) || (error < -tolerance)) {
        camera_sensor->set_shutter_width(camera_sensor, exposure);
        // flush
        camera_fb_t * take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);

        take = sctp_camera_fb_get();
        uint16_t readout = take->buf[px * 2] << 8 | take->buf[px*2 +1];
        sctp_camera_fb_return(take);
        ESP_LOGI(TAG, "%d:%d", iter, readout);
        // ESP_LOGI(TAG, "exposure=%d, max=%d", exposure, max);

        error = setpoint - readout;
        if ((error > tolerance) || (error < -tolerance)) {
            exposure = exposure + kp * error;
            if (exposure < 0) {
                ESP_LOGE(TAG, "exposure too low");
                return ESP_ERR_NOT_FOUND;
            }
        // ESP_LOGI(TAG, "error=%d, new exposure=%d", error, exposure);
        }


        if (exposure > 4096) {
            ESP_LOGE(TAG, "exposure too high");
            return ESP_ERR_NOT_FOUND;
        }
        iter++;
    }

    camera_fb_t * take = sctp_camera_fb_get();
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    *blank_take->readout = take->buf[px * 2] << 8 | take->buf[px*2 +1];
    sctp_camera_fb_return(take);

    sctp_camera_deinit();

    ESP_LOGI(TAG, "exposure=%d", exposure);
    *blank_take->exposure = exposure;
    blank_take->gain = 1;
    
    standby_start();

    return ESP_OK;
};


esp_err_t sctp_sensor_concentration_sample(calibration_t * calibration, uint16_t wavelength, blank_take_t * blank_take, float * sample_take) {
    standby_end();

    sctp_camera_init(&camera_config);
    sensor_t *camera_sensor = sctp_camera_sensor_get();
    camera_sensor->set_row_start(camera_sensor, calibration->row);
    camera_sensor->set_shutter_width(camera_sensor, *blank_take->exposure);

    ESP_LOGI(TAG, "exposure set to %d", *blank_take->exposure);
    
    // flush
    camera_fb_t * take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    
    uint16_t px = round((float) ((wavelength - calibration->bias) / calibration->gain));
    ESP_LOGI(TAG, "wl=%d, px=%d", wavelength, px);

    gpio_set_level( PIN_LAMP_SWITCH, 1);
    take = sctp_camera_fb_get();
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    *sample_take = take->buf[px * 2] << 8 | take->buf[px*2 +1];
    sctp_camera_fb_return(take);

    sctp_camera_deinit();

    standby_start();

    return ESP_OK;
}
