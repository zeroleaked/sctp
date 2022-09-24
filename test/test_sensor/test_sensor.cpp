#include <unity.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include "sctp_sensor.h"
#include "sctp_camera.h"
#include "camera_config.h"
#include "sctp_common_types.h"

//platformio test --environment esp32-s3-devkitc-1 --filter fsm/test_idle -vvv


static const char TAG[] = "test_sensor";

// void spectrum_blank_test() {
//     calibration_t calibration = {};
//     calibration
//     sctp_sensor_spectrum_blank()
// }

void row_search () {
    assert(sctp_camera_init(&camera_config) == ESP_OK);
            vTaskDelay(1000);

    sensor_t *s = sctp_camera_sensor_get();
    s->set_shutter_width(s, 512);


    uint32_t max_val = 0;
    uint16_t max_val_row = 0;
    camera_fb_t * camera_fb = NULL;
    for (int i=0; i<1024; i++) {
        ESP_LOGI(TAG, "i=%d", i);
        uint32_t val = 0;

        if (i%3 == 0) {
            s->set_row_start(s, 0x000C + i);
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
    ESP_LOGI(TAG, "max_row=%d with val=%d", max_val_row, max_val);

    sctp_camera_deinit();

}

void row_print() {
    uint16_t row = 486;
    uint8_t samples = 30;
    float * arr = ( float *) malloc (sizeof (float) * 1280 * 30);
    memset(arr, 0, sizeof(float) * 1280);
    assert(sctp_camera_init(&camera_config) == ESP_OK);

    sensor_t *s = sctp_camera_sensor_get();
    s->set_shutter_width(s, 512);
    s->set_row_start(s, 0x000C + row);

    for (int j=0; j<samples; j++) {
        camera_fb_t * camera_fb = sctp_camera_fb_get(); 
        for (int i=0; i<1280; i++) {
            uint16_t val = (camera_fb->buf[i*2] << 8) | camera_fb->buf[1 + i*2];
            arr[j*1280 + i] = val;
            // ESP_LOGI(TAG, "%d,%d", i, val);
        }
        sctp_camera_fb_return(camera_fb);
    }

    for (int i=0; i<1280; i++) {
        vTaskDelay(10/ portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "%d:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f",
            i, arr[i+1280*0], arr[i+1280*1], arr[i+1280*2], arr[i+1280*3], arr[i+1280*4], arr[i+1280*5], arr[i+1280*6], arr[i+1280*7],
            arr[i+1280*8], arr[i+1280*9], arr[i+1280*10], arr[i+1280*11], arr[i+1280*12], arr[i+1280*13], arr[i+1280*14], arr[i+1280*15],
            arr[i+1280*16], arr[i+1280*17], arr[i+1280*18], arr[i+1280*19], arr[i+1280*20], arr[i+1280*21], arr[i+1280*22], arr[i+1280*23],
            arr[i+1280*24], arr[i+1280*25], arr[i+1280*26], arr[i+1280*27], arr[i+1280*28], arr[i+1280*29]);

    }

    sctp_camera_deinit();

    free(arr);
}

void test_spectrum_blank() {
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;
    calibration.row = 486;

    blank_take_t blank_take;
    blank_take.exposure = 10;
    blank_take.gain = 1;
    blank_take.readout = (float *) malloc (sizeof(float) * calibration.length);

    esp_err_t err = sctp_sensor_spectrum_blank(&calibration, &blank_take);

    TEST_ASSERT_EQUAL(ESP_OK, err);

    assert(err == ESP_OK);

    for (int i=0; i<calibration.length; i++) {
        ESP_LOGI(TAG, "%d:%f", i, blank_take.readout[i]);
    }

    free(blank_take.readout);
}

void test_spectrum() {
    sctp_sensor_init();

    calibration_t calibration;
	calibration.gain = -0.7698064209;
	calibration.bias = 1025.924915;
	calibration.start = 423;
	calibration.length = 392;
    calibration.row = 486;

    blank_take_t blank_take;
    blank_take.exposure = 10;
    blank_take.gain = 1;
    blank_take.readout = (float *) malloc (sizeof(float) * calibration.length);

    esp_err_t err = sctp_sensor_spectrum_blank(&calibration, &blank_take);
    TEST_ASSERT_EQUAL(ESP_OK, err);

    ESP_LOGI(TAG, "blank taken! Taking sample soon....");
    vTaskDelay(5000/ portTICK_PERIOD_MS);
    float * sample_take = (float *) malloc (sizeof(float) * calibration.length);
    err = sctp_sensor_spectrum_sample(&calibration, &blank_take, sample_take);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    ESP_LOGI(TAG, "sample taken");

    for (int i=0; i<calibration.length; i++) {
        ESP_LOGI(TAG, "%d:%f:%f", i, blank_take.readout[i], sample_take[i]);
    }

    free(blank_take.readout);
    free(sample_take);
}

extern "C" {

void app_main();

}

void app_main() {
    UNITY_BEGIN();

    // RUN_TEST(init_test);
    // RUN_TEST(row_search);
    // RUN_TEST(row_print);
    // RUN_TEST(test_spectrum_blank);
    RUN_TEST(test_spectrum);

    UNITY_END();
}