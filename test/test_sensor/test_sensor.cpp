#include <unity.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include <i2cdev.h>

#include "sctp_sensor.h"
#include "sctp_camera.h"
#include "camera_config.h"
#include "sctp_common_types.h"

#include "mt9m001.h"

/***
pio test --filter test_sensor -vvv
***/

static const char TAG[] = "test_sensor";

// void spectrum_blank_test() {
//     calibration_t calibration = {};
//     calibration
//     sctp_sensor_spectrum_blank()
// }
#define PIN_LAMP_SWITCH GPIO_NUM_16

void row_search () {
    i2cdev_init();

    assert(sctp_camera_init(&camera_config) == ESP_OK);

    // lamp pin init
    gpio_config_t conf = {};
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = 1LL << PIN_LAMP_SWITCH;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);

    gpio_set_level( PIN_LAMP_SWITCH, 1);

    // vTaskDelay(5000 / portTICK_PERIOD_MS);

    sensor_t *s = sctp_camera_sensor_get();
    s->set_shutter_width(s, 250);


    uint32_t max_val = 0;
    uint16_t max_val_row = 0;
    camera_fb_t * camera_fb = NULL;
    for (int i=0; i<1024; i++) {
        if (!(i % 50))
            ESP_LOGI(TAG, "i=%d", i);
        uint32_t val = 0;

        if (i%3 == 0) {
            s->set_row_start(s, 0x000C + i);
            camera_fb_t * take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);
            take = sctp_camera_fb_get();
            sctp_camera_fb_return(take);

            camera_fb = sctp_camera_fb_get();
            assert(camera_fb != NULL);
        }

        uint16_t row_offset = 1280*(i%3);
        // ESP_LOGI(TAG, "%d to %d", row_offset, row_offset+1279);
        for (int j=0; j<1280; j++) {
            val += (camera_fb->buf[j*2 + row_offset] << 8) | camera_fb->buf[1 + j*2 + row_offset];
        }

        if (val > max_val) {
            max_val = val;
            max_val_row = i;
        }

        if (i%3 == 2) {
            sctp_camera_fb_return(camera_fb);
        }
    }
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    ESP_LOGI(TAG, "max_row=%d with val=%d", max_val_row, max_val);

    sctp_camera_deinit();

}

void row_print() {
    i2cdev_init();
    uint16_t row = 490;
    uint8_t samples = 30;
    float * arr = ( float *) malloc (sizeof (float) * 1280 * samples);
    memset(arr, 0, sizeof(float) * 1280 * samples);
    assert(sctp_camera_init(&camera_config) == ESP_OK);


    // lamp pin init
    gpio_config_t conf = {};
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = 1LL << PIN_LAMP_SWITCH;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);
    gpio_set_level( PIN_LAMP_SWITCH, 0);


    sensor_t *s = sctp_camera_sensor_get();
    // s->set_gain(s, 8);
    s->set_shutter_width(s, 6000);
    s->set_row_start(s, 0x000C + row);

    camera_fb_t * take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);

    ESP_LOGI(TAG, "lamp config done. turning on");

    gpio_set_level( PIN_LAMP_SWITCH, 1);
    vTaskDelay(3000/ portTICK_PERIOD_MS);
    for (int j=0; j<samples; j++) {
        if (j < 2) {
            ESP_LOGI(TAG, "j=%d", j);
        }
        camera_fb_t * camera_fb = sctp_camera_fb_get(); 
        for (int i=0; i<1280; i++) {
            uint16_t val = (camera_fb->buf[i*2] << 8) | camera_fb->buf[1 + i*2];
            arr[j*1280 + i] = val;
            // ESP_LOGI(TAG, "%d,%d", i, val);
        }
        sctp_camera_fb_return(camera_fb);
    }
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    ESP_LOGI(TAG, "turned off");

    for (int i=0; i<1280; i++) {
        vTaskDelay(10/ portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "%d:%f",
            i, arr[i+1280*0]);
        // ESP_LOGI(TAG, "%d:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f",
        //     i, arr[i+1280*0], arr[i+1280*1], arr[i+1280*2], arr[i+1280*3], arr[i+1280*4], arr[i+1280*5], arr[i+1280*6], arr[i+1280*7],
        //     arr[i+1280*8], arr[i+1280*9], arr[i+1280*10], arr[i+1280*11], arr[i+1280*12], arr[i+1280*13], arr[i+1280*14], arr[i+1280*15],
        //     arr[i+1280*16], arr[i+1280*17], arr[i+1280*18], arr[i+1280*19], arr[i+1280*20], arr[i+1280*21], arr[i+1280*22], arr[i+1280*23],
        //     arr[i+1280*24], arr[i+1280*25], arr[i+1280*26], arr[i+1280*27], arr[i+1280*28], arr[i+1280*29]);

    }

    sctp_camera_deinit();

    free(arr);
}

void test_spectrum_blank() {
    i2cdev_init();
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -1.051282051;
	calibration.bias = 1175.012821;
	calibration.start = 250;
	calibration.length = 500;
    calibration.row = 493;

    blank_take_t blank_take;
    blank_take.exposure = (uint16_t *) malloc (sizeof(uint16_t) * calibration.length);
    blank_take.exposure[0] = 7800;
    blank_take.gain = 1;
    blank_take.readout = (float *) malloc (sizeof(float) * calibration.length);

    esp_err_t err = sctp_sensor_spectrum_blank(&calibration, &blank_take);

    TEST_ASSERT_EQUAL(ESP_OK, err);

    assert(err == ESP_OK);

    for (int i=0; i<calibration.length; i++) {
        ESP_LOGI(TAG, "%d:%f:%d", i, blank_take.readout[i], blank_take.exposure[i]);
    }

    free(blank_take.readout);
    free(blank_take.exposure);
}

uint16_t exp_dummy[] = {8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,8000,6600,6600,6600,6600,6600,6600,6600,6600,5368,5368,5368,5368,5368,5368,5368,4912,4912,4464,4464,4464,4464,3872,3872,3872,3352,3352,3352,3352,3352,3352,3352,2800,2800,2800,2800,2800,2800,2800,2800,2475,2475,2475,2475,2195,2195,2195,2195,2195,2195,1890,1890,1890,1890,1555,1555,1555,1555,1555,1555,1555,1555,1555,1555,1555,1555,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1311,1252,1200,1200,1135,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1060,1002,1002,1002,1002,941,928,900,881,861,849,799,793,781,769,769,769,769,769,763,763,763,763,763,737,716,697,697,684,684,684,684,671,671,645,639,606,573,546,546,517,505,478,478,478,478,478,478,478,478,478,464,441,435,435,435,407,407,393,387,387,387,387,387,367,367,355,355,349,349,320,314,314,308,302,302,302,302,302,302,302,296,296,296,296,296,296,284,284,284,278,278,278,278,266,266,266,266,266,266,266,266,266,266,260,253,253,253,253,246,239,239,239,239,233,233,233,233,233,233,227,227,227,227,227,227,227,221,215,215,215,215,215,215,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,209,214,214,214,214,214,214,214,219,219,230,230,242,242,247,247,247,258,269,269,269,269,269,269,269,269,269,274,274,274,274,274,274,274,279,279,285,296,301,301,301,301,301,301,314,314,319,319,324,324,324,324,329,329,329,329,334,345,350,355,355,360,365,365,365,375,402,402,413,423,433,433,443,443,460,470,475,475,475,480,491,491,507,512,517,517,529,529,540,545,551,583,583,583,583,583,604,604,620,630,641,651,651,656,667,694,717,727,732,757,787,792,831,831,831,831,831,831,831,831,868,868,888,888,888,888,909,919,952,1002,1002,1002,1002,1073,1130,1130,1130,1130,1130,1130,1130,1183,1183,1183,1183,1235,1235,1235,1235,1290,1290,1290,1342,1342,1342,1342,1342,1393,1393};

void test_spectrum() {
    i2cdev_init();
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -1.051282051;
	calibration.bias = 1175.012821;
	calibration.start = 250;
	calibration.length = 500;
    calibration.row = 493;

    blank_take_t blank_take;
    blank_take.exposure = (uint16_t *) malloc (sizeof(uint16_t) * calibration.length);
    blank_take.exposure[0] = 7800;
    blank_take.gain = 1;
    blank_take.readout = (float *) malloc (sizeof(float) * calibration.length);

    for (int i=0; i<calibration.length; i++) {
        blank_take.exposure[i] = exp_dummy[i];
    }

    // esp_err_t err = sctp_sensor_spectrum_blank(&calibration, &blank_take);



    esp_err_t err = ESP_OK;

    for (int i=0; i< calibration.length; i++) {
        blank_take.readout[i] = 1024;
    }

    TEST_ASSERT_EQUAL(ESP_OK, err);

    assert(err == ESP_OK);

    for (int i=0; i<calibration.length; i++) {
        ESP_LOGI(TAG, "%d:%f:%d", i, blank_take.readout[i], blank_take.exposure[i]);
    }

    ESP_LOGI(TAG, "blank taken! Taking sample soon....");
    // vTaskDelay(300000/ portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Taking now!");
    float * sample_take = (float *) malloc (sizeof(float) * calibration.length);
    err = sctp_sensor_spectrum_sample(&calibration, &blank_take, sample_take);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    ESP_LOGI(TAG, "sample taken");

    for (int i=0; i<calibration.length; i++) {
        ESP_LOGI(TAG, "%d:%f:%f:%d", i, blank_take.readout[i], sample_take[i], blank_take.exposure[i]);
        vTaskDelay(10 /portTICK_PERIOD_MS);
    }

    free(blank_take.readout);
    free(blank_take.exposure);
    free(sample_take);
}

void test_quant_blank() {
    i2cdev_init();
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;
    calibration.row = 489;

    blank_take_t blank_take;
    uint16_t exposure = 100;
    blank_take.exposure = &exposure;
    float readout = 0;
    blank_take.readout = &readout;

    esp_err_t err = sctp_sensor_concentration_blank(&calibration, 520, &blank_take);
    TEST_ASSERT_EQUAL(ESP_OK, err);

    ESP_LOGI(TAG, "blank taken!");
    ESP_LOGI(TAG, "exposure = %d", *blank_take.exposure);
    ESP_LOGI(TAG, "blank = %f", *blank_take.readout);
}

void test_quant_sample() {
    i2cdev_init();
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;
    calibration.row = 492;

    blank_take_t blank_take;
    uint16_t exposure = 8000;
    blank_take.exposure = &exposure;
    float blank_readout = 465;
    blank_take.readout = &blank_readout;

    uint16_t wavelength = 520;

    float sample_readout = 0;
    esp_err_t err = sctp_sensor_concentration_sample(&calibration, wavelength, &blank_take, &sample_readout);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    ESP_LOGI(TAG, "sample taken");

    ESP_LOGI(TAG, "%f:%f:%d", *blank_take.readout, sample_readout, *blank_take.exposure);

}

void test_quant() {
    i2cdev_init();
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;
    calibration.row = 492;

    blank_take_t blank_take;
    uint16_t exposure = 10;
    blank_take.exposure = &exposure;
    blank_take.gain = 1;
    float blank_readout = 0;
    blank_take.readout = &blank_readout;

    uint16_t wavelength = 520;

    esp_err_t err = sctp_sensor_concentration_blank(&calibration, wavelength, &blank_take);
    TEST_ASSERT_EQUAL(ESP_OK, err);

    ESP_LOGI(TAG, "blank taken! Taking sample soon....");
    vTaskDelay(10000/ portTICK_PERIOD_MS);
    float sample_readout = 0;
    err = sctp_sensor_concentration_sample(&calibration, wavelength, &blank_take, &sample_readout);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    ESP_LOGI(TAG, "sample taken");

    ESP_LOGI(TAG, "%f:%f:%d", *blank_take.readout, sample_readout, *blank_take.exposure);

}

void test_exposure() {
    i2cdev_init();
    sctp_camera_init(&camera_config);

    calibration_t calibration;
	calibration.gain = -0.7666855524;
	calibration.bias = 1013.975014;
	calibration.start = 409;
	calibration.length = 393;
    calibration.row = 499;

    sensor_t *s = sctp_camera_sensor_get();
    s->set_row_start(s, 0x000C + calibration.row);

    uint16_t * buffer = (uint16_t *) malloc(sizeof(uint16_t) * calibration.length * 30);

    uint16_t * sample_total = (uint16_t *) malloc(sizeof(uint16_t) * calibration.length);
    memset(sample_total, 0, sizeof(uint16_t) * calibration.length);

    for (int i=0; i<20; i++) {
        s->set_shutter_width(s, (i+1)*100);

        camera_fb_t * take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);

        for (int k=0; k<30; k++) {
            take = sctp_camera_fb_get();

            

            for (int j=0; j<calibration.length; j++) {
                uint16_t readout = take->buf[(calibration.start + calibration.length -1 -j) * 2] << 8 | take->buf[(calibration.start + calibration.length -1 -j) * 2 + 1];
                sample_total[j] += readout;
                buffer[i*calibration.length+ calibration.length -1 - j] = readout;
            }
            sctp_camera_fb_return(take);
        }



    }



}

void test_lamp() {
    i2cdev_init();
    uint16_t row = 472;
    uint16_t samples = 200;
    uint32_t * arr = ( uint32_t *) malloc (sizeof (uint32_t) * samples);
    timeval * time = ( timeval *) malloc (sizeof (timeval) * samples);
    memset(arr, 0, sizeof(uint32_t) * samples);
    assert(sctp_camera_init(&camera_config) == ESP_OK);


    // lamp pin init
    gpio_config_t conf = {};
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.mode = GPIO_MODE_OUTPUT;
    conf.pin_bit_mask = 1LL << PIN_LAMP_SWITCH;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&conf);
    gpio_set_level( PIN_LAMP_SWITCH, 0);


    sensor_t *s = sctp_camera_sensor_get();
    s->set_shutter_width(s, 1108);
    s->set_row_start(s, 0x000C + row);

    camera_fb_t * take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);
    take = sctp_camera_fb_get();
    sctp_camera_fb_return(take);

    ESP_LOGI(TAG, "lamp config done. turning on");


    uint16_t pixel = 472;
    gpio_set_level( PIN_LAMP_SWITCH, 1);
    for (int j=0; j<samples; j++) {
        // ESP_LOGI(TAG, "j=%d", j);
        camera_fb_t * camera_fb = sctp_camera_fb_get(); 
        arr[j] = (camera_fb->buf[pixel*2] << 8) | camera_fb->buf[1 + pixel*2];
        ESP_LOGI(TAG, "%d:%d", j, arr[j]);
        time[j]= camera_fb->timestamp;
        sctp_camera_fb_return(camera_fb);
        vTaskDelay(750 / portTICK_PERIOD_MS);
    }
    gpio_set_level( PIN_LAMP_SWITCH, 0);
    ESP_LOGI(TAG, "turned off");

    for (int i=0; i<samples; i++) {
        vTaskDelay(10/ portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "%ld:%d", time[i].tv_sec*1000+time[i].tv_usec/1000, arr[i]);
    }

    sctp_camera_deinit();

    free(arr);
    free(time);
}

void test_linearity() {
    i2cdev_init();
    uint16_t row = 471;
    // uint16_t points[100];
    assert(sctp_camera_init(&camera_config) == ESP_OK);

    uint16_t shutter_start = 2000;
    uint16_t shutter_step = 100;

    sensor_t *s = sctp_camera_sensor_get();
    s->set_row_start(s, 0x000C + row);

    uint16_t col = 0;
    for (int i=0; i<100; i++) {
        uint16_t exposure = shutter_start + i * shutter_step;
        s->set_shutter_width(s, exposure);
        camera_fb_t * take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);
        take = sctp_camera_fb_get();
        sctp_camera_fb_return(take);

        uint16_t total = 0;
        for (int j=0; j<1; j++) {
            take = sctp_camera_fb_get();
            
            if ((i==0) && (j==0)) {
                uint16_t col_val = 0;
                for (int k=0; k<1000; k++) {
                    uint16_t val = (take->buf[k*2] << 8) | take->buf[1 + k*2];
                    if (val > col_val) {
                        col_val = val;
                        col = k;
                    }
                }
                ESP_LOGI(TAG, "col=%d", col);
            }

            uint16_t val = (take->buf[col*2] << 8) | take->buf[1 + col*2];
            total += val;

            sctp_camera_fb_return(take);
        }

        // points[i] = total;
        ESP_LOGI(TAG, "%d:%d", exposure, total);
    }

    sctp_camera_deinit();

}

void test_check() {
    i2cdev_init();

    sctp_sensor_init();

    calibration_t c;
	c.gain = -0.7698064209;
	c.bias = 1025.924915;
	c.start = 423;
	c.length = 392;
    c.row = 492;

    uint16_t result = 0;

    for (int i=0; i<9; i++) {
        sctp_sensor_check(&c, &result);
        ESP_LOGI(TAG, "CHECK: %d", result);
        vTaskDelay(10000/portTICK_PERIOD_MS);
    }
}

extern "C" {

void app_main();
}

void app_main() {
    UNITY_BEGIN();

    // RUN_TEST(row_search);
    RUN_TEST(row_print);

    // RUN_TEST(test_spectrum_blank);
    // RUN_TEST(test_spectrum);
    // RUN_TEST(test_quant_blank);
    // RUN_TEST(test_quant_sample);
    // RUN_TEST(test_quant);


    // RUN_TEST(test_lamp);
    // RUN_TEST(test_exposure);
    RUN_TEST(test_linearity);

    // RUN_TEST(test_check);

    UNITY_END();
}