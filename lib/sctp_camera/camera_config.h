#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 2
#define CAM_PIN_SIOD 11
#define CAM_PIN_SIOC 10

#define CAM_PIN_D9 19
#define CAM_PIN_D8 8
#define CAM_PIN_D7 18
#define CAM_PIN_D6 17
#define CAM_PIN_D5 16
#define CAM_PIN_D4 15
#define CAM_PIN_D3 7
#define CAM_PIN_D2 6
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 20
#define CAM_PIN_HREF 9
#define CAM_PIN_PCLK 1

static camera_config_t camera_config = {
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,

        .pin_d9 = CAM_PIN_D9,
        .pin_d8 = CAM_PIN_D8,
        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
        // .xclk_freq_hz = 20000000,
        .xclk_freq_hz = 10000000,
        // .xclk_freq_hz = 5000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .fb_count = 2,       //if more than one, i2s runs in continuous mode. Use only with JPEG
    };