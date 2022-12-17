#include <esp_log.h>
#include <math.h>

#include "spec_sample_state.h"
#include "spec_blank_state.h"
#include "spec_result_state.h"
#include "sctp_lcd.h"
#include "sctp_sensor.h"

#define CURSOR_NEXT 0
#define CURSOR_CHECK 1
#define CURSOR_CANCEL 2
#define CURSOR_NULL 3

#define SUBSTATE_WAITING 0
#define SUBSTATE_SAMPLING 1

static const char TAG[] = "spec_sample_state";

static const float filter[] = {-0.0001443369656572586, -0.00015883599548364635, -0.00017281759479082594, -0.0001862671719858604, -0.00019914643084690867, -0.00021139047985650638, -0.00022290558331426128, -0.00023356762929193714, -0.0002432213781947426, -0.00025168054305240974, -0.00025872873885484006, -0.00026412132346269866, -0.00026758813707415023, -0.00026883713114091227, -0.0002675588612379104, -0.00026343180194790365, -0.00025612842557766534, -0.00024532197072979045, -0.00023069381166624568, -0.0002119413252633764, -0.00018878613941122584, -0.00016098263517813166, -0.00012832656515426343, -9.066364229535177e-05, -4.789794747860976e-05, 1.96685205603282e-19, 5.2985666498563115e-05, 0.00011093357959528307, 0.0001736312090990405, 0.00024077380207702115, 0.0003119603126977841, 0.00038669055756290126, 0.0004643637148028148, 0.0005442782706549305, 0.0006256335006688044, 0.0007075325542705798, 0.0007889871913715927, 0.0008689241982567923, 0.0009461934873939678, 0.001019577862343058, 0.0010878044049123657, 0.0011495574174161956, 0.0012034928286576998, 0.0012482539484194558, 0.0012824884321228323, 0.0013048662952434994, 0.0013140987963663345, 0.001308957988739286, 0.001288296723138276, 0.0012510688700605069, 0.0011963495169750153, 0.0011233548868032673, 0.0010314617171752812, 0.0009202258364706285, 0.0007893996723354307, 0.0006389484313540024, 0.0004690646948950565, 0.0002801811858538109, 7.298147403651834e-05, -0.00015159159579531322, -0.00039232994980909023, -0.000647756979145137, -0.0009161270312371273, -0.001195427819892775, -0.0014833858457459286, -0.0017774748868884052, -0.0020749275860974495, -0.002372750126458415, -0.002667739951730405, -0.002956506451920681, -0.003235494498639715, -0.0035010106793290103, -0.0037492520448167556, -0.003976337151288449, -0.004178339146078709, -0.004351320617102407, -0.004491369898635028, -0.0045946385018880076, -0.004657379317741136, -0.004675985221395179, -0.0046470276948613895, -0.004567295073338486, -0.004433830015826348, -0.004243965798927691, -0.0039953610357845535, -0.003686032429525468, -0.0033143851824513847, -0.002879240698400927, -0.0023798612361949365, -0.0018159711966021818, -0.0011877747536787448, -0.0004959695733525206, 0.0002582436025546133, 0.0010731556903974245, 0.0019465484595541818, 0.002875698287734282, 0.003857384531728479, 0.004887902522649758, 0.0059630811480381385, 0.007078304936293067, 0.008228540512262978, 0.009408367246978877, 0.010612011879975809, 0.01183338684990347, 0.013066132028668293, 0.014303659516636256, 0.015539201121893512, 0.016765858115613024, 0.017976652828575416, 0.019164581631164534, 0.020322668820981684, 0.0214440209288253, 0.0225218809453432, 0.023549681967304826, 0.024521099764228414, 0.025430103773043347, 0.02627100604052348, 0.027038507650289485, 0.027727742193087417, 0.02833431586559185, 0.028854343813889807, 0.029284482372760004, 0.029621956890509876, 0.029864584871067953, 0.0300107942098106, 0.030059636346758567, 0.0300107942098106, 0.029864584871067953, 0.029621956890509876, 0.02928448237276, 0.028854343813889804, 0.028334315865591847, 0.027727742193087414, 0.027038507650289485, 0.02627100604052348, 0.025430103773043344, 0.024521099764228414, 0.023549681967304826, 0.0225218809453432, 0.021444020928825296, 0.020322668820981684, 0.01916458163116453, 0.017976652828575416, 0.01676585811561302, 0.015539201121893512, 0.014303659516636254, 0.013066132028668293, 0.01183338684990347, 0.010612011879975809, 0.009408367246978874, 0.008228540512262976, 0.007078304936293065, 0.005963081148038138, 0.004887902522649756, 0.0038573845317284787, 0.0028756982877342816, 0.0019465484595541816, 0.0010731556903974245, 0.00025824360255461326, -0.0004959695733525204, -0.0011877747536787448, -0.0018159711966021811, -0.0023798612361949365, -0.0028792406984009264, -0.003314385182451384, -0.0036860324295254674, -0.0039953610357845535, -0.004243965798927691, -0.004433830015826346, -0.004567295073338485, -0.0046470276948613895, -0.004675985221395179, -0.004657379317741135, -0.004594638501888007, -0.004491369898635025, -0.004351320617102406, -0.004178339146078709, -0.003976337151288447, -0.0037492520448167543, -0.0035010106793290103, -0.003235494498639715, -0.0029565064519206795, -0.0026677399517304034, -0.002372750126458415, -0.0020749275860974495, -0.0017774748868884045, -0.001483385845745928, -0.001195427819892775, -0.0009161270312371271, -0.0006477569791451367, -0.00039232994980909007, -0.00015159159579531322, 7.298147403651833e-05, 0.0002801811858538108, 0.0004690646948950562, 0.0006389484313540019, 0.0007893996723354305, 0.0009202258364706283, 0.0010314617171752808, 0.0011233548868032665, 0.0011963495169750153, 0.0012510688700605066, 0.0012882967231382756, 0.001308957988739285, 0.0013140987963663345, 0.0013048662952434994, 0.0012824884321228317, 0.0012482539484194545, 0.0012034928286576998, 0.0011495574174161956, 0.001087804404912365, 0.001019577862343057, 0.0009461934873939678, 0.0008689241982567923, 0.0007889871913715922, 0.0007075325542705793, 0.0006256335006688035, 0.0005442782706549305, 0.00046436371480281446, 0.000386690557562901, 0.00031196031269778364, 0.00024077380207702115, 0.0001736312090990404, 0.000110933579595283, 5.298566649856305e-05, 1.96685205603282e-19, -4.789794747860976e-05, -9.066364229535173e-05, -0.00012832656515426324, -0.00016098263517813166, -0.00018878613941122584, -0.00021194132526337623, -0.0002306938116662455, -0.00024532197072979045, -0.00025612842557766534, -0.00026343180194790354, -0.00026755886123791024, -0.00026883713114091195, -0.00026758813707415023, -0.00026412132346269855, -0.0002587287388548399, -0.0002516805430524096, -0.0002432213781947426, -0.00023356762929193698, -0.00022290558331426115, -0.00021139047985650625, -0.00019914643084690867, -0.0001862671719858604, -0.00017281759479082594, -0.00015883599548364625, -0.0001443369656572586};
typedef struct {
    QueueHandle_t report_queue;
    calibration_t *calibration;
    blank_take_t *blank_take;
    float *sample_take;
    float *absorbance;
    uint8_t * percentage;
} taskParam_t;

void SpecSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_CHECK;
    check_result = (uint16_t *)malloc(sizeof(uint16_t));
    *check_result = 0;
    percentage = (uint8_t*)malloc(sizeof(uint8_t));
    *percentage = 0;
    sctp_lcd_spec_sample_waiting(cursor, *check_result);
}

void filterAbsorbance(float * input, float * output, int length) {
    ESP_LOGI(TAG, "zero init");
    for (int i=0; i< length; i++) {
        output[i] = 0;
    }

    ESP_LOGI(TAG, "start");
    for (int i=0; i < length; i++) {
        for (int j=0; j<251; j++) {
            int pseu_i = i - 125 + j;
            if ((pseu_i >= 0) && (pseu_i<length)) {
                output[i] += filter[j] * input[pseu_i];
            }
        }
    }
    ESP_LOGI(TAG, "filter end");

}

static void takeSpectrumSample(void * pvParameters) {
    blank_take_t * blank_take = ((taskParam_t *) pvParameters)->blank_take;
    float * sample_take = ((taskParam_t *) pvParameters)->sample_take;
    calibration_t * calibration = ((taskParam_t *) pvParameters)->calibration;
    uint8_t * percentage = ((taskParam_t *) pvParameters)->percentage;

	esp_err_t report = sctp_sensor_spectrum_sample(calibration, blank_take, sample_take, percentage);

    float * absorbance = ((taskParam_t *) pvParameters)->absorbance;

	// castings
	float * blank_buffer = blank_take->readout;
	float * sample_buffer = sample_take;
	for (int i=0; i < calibration->length; i++) {
		float transmission = sample_buffer[i]/blank_buffer[i];
		absorbance[i] = -log10(transmission);
	}

    float * unfiltered = (float *) malloc(sizeof(float) * calibration->length);
    memcpy(unfiltered, absorbance, sizeof(float) * calibration->length);
	filterAbsorbance(unfiltered, absorbance, calibration->length);
    free(unfiltered);

    QueueHandle_t report_queue = ((taskParam_t *) pvParameters)->report_queue;
    assert(xQueueSend(report_queue, &report, 0) == pdTRUE);
	vTaskDelete( NULL );

}

void SpecSample::okay(Sctp* sctp)
{
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    sctp_lcd_spec_sample_clear(cursor);
                    sctp_lcd_spec_sample_waiting(cursor, *check_result);
                    cursor = CURSOR_NULL;
                    sctp_lcd_spec_sample_sampling(cursor, *percentage);

                    report_queue = xQueueCreate(1, sizeof(esp_err_t));
                    substate = SUBSTATE_SAMPLING;
                    
                    assert(sctp->sample_take == NULL);
                    sctp->sample_take = (float *) malloc (sizeof(float) * sctp->calibration.length);
                    assert(sctp->absorbance == NULL);
                    sctp->absorbance = (float *) malloc (sizeof(float) * sctp->calibration.length);
                    taskParam = malloc (sizeof(taskParam_t));
                	((taskParam_t *) taskParam)->report_queue = report_queue;
                	((taskParam_t *) taskParam)->calibration = &sctp->calibration;
                	((taskParam_t *) taskParam)->blank_take = sctp->blank_take;
                	((taskParam_t *) taskParam)->sample_take = sctp->sample_take;
                	((taskParam_t *) taskParam)->absorbance = sctp->absorbance;
                	((taskParam_t *) taskParam)->percentage = percentage;

                    xTaskCreatePinnedToCore(takeSpectrumSample, "takeSpectrumSample", 4096, taskParam, 4, &taskHandle, 1);
                    break;
                }
                case CURSOR_CHECK:
                {
                    sctp_sensor_check(&sctp->calibration, check_result);
                    sctp_lcd_spec_sample_waiting(cursor, *check_result);
                    break;
                }
                case CURSOR_CANCEL: {
                    free(sctp->blank_take);
                    sctp->blank_take = NULL;
                    sctp->setState(SpecBlank::getInstance());
                    break;
                }
            }
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_CANCEL: {
                    break;
                }
            }
        }
    }
}

void SpecSample::arrowLeft(Sctp* sctp)
{
	sctp_lcd_spec_sample_clear(cursor);
    switch (substate) {
        case SUBSTATE_WAITING: {
            switch (cursor) {
                case CURSOR_NEXT: {
                    cursor = CURSOR_CHECK;
                    break;
                }
                case CURSOR_CHECK: {
                    cursor = CURSOR_CANCEL;
                    break;
                }
                case CURSOR_CANCEL: {
                    cursor = CURSOR_NEXT;
                    break;
                }
            }
            sctp_lcd_spec_sample_waiting(cursor, *check_result);
            break;
        }
        case SUBSTATE_SAMPLING: {
            switch (cursor) {
                case CURSOR_NULL: {
                    cursor = CURSOR_CANCEL;
                    sctp_lcd_spec_sample_sampling(cursor, *percentage);
                    break;
                }
            }
        }
    }
}

void SpecSample::arrowRight(Sctp* sctp)
{
    sctp_lcd_spec_sample_clear(cursor);
    if(substate == SUBSTATE_WAITING) {
        switch (cursor)
        {
            case CURSOR_NEXT:
            {
                cursor = CURSOR_CANCEL;
                break;
            }
            case CURSOR_CHECK:
            {
                cursor = CURSOR_NEXT;
                break;
            }
            case CURSOR_CANCEL:
            {
                cursor = CURSOR_CHECK;
                break;
            }
        }
    sctp_lcd_spec_sample_waiting(cursor, *check_result);
    }
}

void SpecSample::refreshLcd(Sctp* sctp, command_t command) {
    // if (command == SPECTRUM_SAMPLE) { // sample taken
	// 	sctp->setState(SpecResult::getInstance());
    // }

    if (substate == SUBSTATE_SAMPLING) {
        esp_err_t report;
        if (xQueueReceive(report_queue, &report, 0) == pdTRUE) {
            if (report == ESP_OK) {
                free(taskParam);
                taskParam = NULL;
                vQueueDelete(report_queue);
                report_queue = NULL;
                sctp->setState(SpecResult::getInstance());
            }
        } else {
           sctp_lcd_spec_blank_sampling_percentage(*percentage);
        }
    }
}

void SpecSample::exit(Sctp *sctp)
{
    free(check_result);
    free(percentage);
}

SctpState& SpecSample::getInstance()
{
	static SpecSample singleton;
	return singleton;
}