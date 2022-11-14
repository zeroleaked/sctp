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

static const float filter[] = {0.00020368284868270555,0.00019563071352854215,0.0001686784773140982,0.0001246696600826432,6.690987225286152e-05,-3.2593656802015404e-19,-7.042914643506726e-05,-0.00013808017993524027,-0.00019644258123878195,-0.00023931817755592778,-0.00026138859597910866,-0.00025878241844667986,-0.0002295893826108951,-0.00017426262525861477,-9.584966202879159e-05,1.5920365449700757e-18,0.00010528741447325813,0.0002101907901797077,0.000303851133894147,0.0003753698917855028,0.00041493993258322247,0.00041500417406639814,0.00037131778176713076,0.0002837837070739382,0.00015693982690036397,-6.542026979635504e-19,-0.000173609972164217,-0.0003472300655424351,-0.0005024084867439328,-0.0006207013120218451,-0.0006856689348230941,-0.0006848687605708006,-0.0006116221335241775,-0.0004663352387325012,-0.0002571808792589374,9.618426596842277e-19,0.00028264493012162813,0.0005632444123189114,0.0008118296310760335,0.0009989682830326093,0.0010989875626435989,0.001093091991917129,0.0009720249080020721,0.00073794099946616,0.0004052144600609396,-1.3412891702771707e-18,-0.0004415152318752951,-0.0008760929034289191,-0.0012574410021776432,-0.0015408818069105972,-0.0016882430396678912,-0.0016724640512152807,-0.001481398523331077,-0.001120340313833688,-0.0006128990343624829,1.768700209975177e-18,0.0006630331454877866,0.0013111527837206221,0.001875658351062833,0.0022911275381819053,0.002502539337023606,0.002471850454409367,0.0021832899563251807,0.0016467186350256635,0.0008985562476456529,-2.217219971246172e-18,-0.0009674775257908407,-0.0019090783123243554,-0.0027255285109317362,-0.0033230424916557634,-0.003623444996114687,-0.003573412019639527,-0.0031518105133463963,-0.0023742435689510853,-0.0012941351917387857,2.658666307949206e-18,0.0013910885741275152,0.002743427053520318,0.003915228136927248,0.004772685182723508,0.005204227438751975,0.005133530349908069,0.004529875602350639,0.003414629705266264,0.0018629189931452673,-3.0653015232231662e-18,-0.0020076782593933065,-0.003966194557197442,-0.005671622992518332,-0.0069297736219522356,-0.007576411247856559,-0.0074959930439852315,-0.006636982551184442,-0.005022004124018419,-0.0027514885729597767,3.4115752309926832e-18,0.002994917957929623,0.005950837315344926,0.008564253486206626,0.010538264166169098,0.01161180411689283,0.01158783682540365,0.010357845839250498,0.007920152007868954,0.004390001975728543,-3.675729780692981e-18,-0.004909758898927367,-0.009911616230342406,-0.014519201025377373,-0.018223231299358093,-0.020531595729089733,-0.021010555014753837,-0.019323669569270063,-0.015265113811517957,-0.008784381552908657,3.841167370473914e-18,0.010799294515552853,0.02316647494469301,0.03652072782031607,0.05018298846186051,0.06342001332411688,0.07549374754998524,0.08571225537723538,0.09347829394860314,0.09833174689861934,0.09998258454434822,0.09833174689861934,0.09347829394860314,0.08571225537723538,0.07549374754998524,0.06342001332411688,0.050182988461860506,0.03652072782031606,0.02316647494469301,0.010799294515552853,3.841167370473914e-18,-0.008784381552908657,-0.015265113811517956,-0.019323669569270063,-0.021010555014753834,-0.020531595729089733,-0.01822323129935809,-0.014519201025377371,-0.009911616230342405,-0.004909758898927367,-3.675729780692981e-18,0.004390001975728543,0.007920152007868952,0.010357845839250498,0.011587836825403649,0.011611804116892829,0.010538264166169094,0.008564253486206626,0.005950837315344924,0.0029949179579296225,3.4115752309926825e-18,-0.0027514885729597763,-0.005022004124018418,-0.0066369825511844414,-0.00749599304398523,-0.007576411247856558,-0.006929773621952233,-0.005671622992518332,-0.003966194557197441,-0.002007678259393306,-3.065301523223166e-18,0.0018629189931452673,0.003414629705266264,0.004529875602350638,0.0051335303499080685,0.005204227438751975,0.004772685182723508,0.003915228136927247,0.002743427053520317,0.0013910885741275146,2.6586663079492054e-18,-0.0012941351917387857,-0.0023742435689510844,-0.003151810513346395,-0.003573412019639527,-0.003623444996114687,-0.003323042491655762,-0.0027255285109317345,-0.0019090783123243554,-0.0009674775257908405,-2.2172199712461713e-18,0.0008985562476456526,0.0016467186350256635,0.0021832899563251802,0.0024718504544093663,0.002502539337023605,0.0022911275381819053,0.0018756583510628325,0.0013111527837206215,0.0006630331454877861,1.7687002099751755e-18,-0.0006128990343624828,-0.0011203403138336876,-0.0014813985233310765,-0.0016724640512152797,-0.0016882430396678912,-0.0015408818069105967,-0.0012574410021776426,-0.0008760929034289185,-0.0004415152318752951,-1.3412891702771707e-18,0.00040521446006093936,0.0007379409994661592,0.0009720249080020721,0.001093091991917129,0.0010989875626435982,0.000998968283032608,0.0008118296310760335,0.0005632444123189114,0.00028264493012162797,9.61842659684227e-19,-0.00025718087925893706,-0.0004663352387325012,-0.000611622133524177,-0.0006848687605708001,-0.0006856689348230932,-0.0006207013120218451,-0.0005024084867439325,-0.0003472300655424349,-0.00017360997216421677,-6.542026979635504e-19,0.00015693982690036397,0.0002837837070739381,0.0003713177817671302,0.00041500417406639814,0.00041493993258322247,0.00037536989178550245,0.00030385113389414675,0.0002101907901797077,0.00010528741447325813,1.592036544970075e-18,-9.584966202879154e-05,-0.00017426262525861458,-0.0002295893826108951,-0.0002587824184466797,-0.0002613885959791085,-0.00023931817755592764,-0.00019644258123878195,-0.00013808017993524016,-7.042914643506722e-05,-3.2593656802015385e-19,6.690987225286152e-05,0.0001246696600826432,0.0001686784773140982,0.00019563071352854202,0.00020368284868270555};

typedef struct {
    QueueHandle_t report_queue;
    calibration_t * calibration;
    blank_take_t * blank_take;
    float * sample_take;
    float * absorbance;
} taskParam_t;

void SpecSample::enter(Sctp* sctp)
{
	sctp_lcd_clear();
    substate = SUBSTATE_WAITING;
	cursor = CURSOR_CHECK;
    check_result = (uint16_t *)malloc(sizeof(uint16_t));
    *check_result = 0;
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

	esp_err_t report = sctp_sensor_spectrum_sample(calibration, blank_take, sample_take);

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
                    sctp_lcd_spec_sample_sampling(cursor);

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
                    vTaskDelete(taskHandle);
                    taskHandle = NULL;
                    vQueueDelete(report_queue);
                    report_queue = NULL;

                    free(sctp->sample_take);
                    sctp->sample_take = NULL;
                    free(sctp->absorbance);
                    sctp->absorbance = NULL;
                    free(taskParam);
                    taskParam = NULL;

                    substate = SUBSTATE_WAITING;
                    sctp_lcd_spec_sample_waiting(cursor, *check_result);
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
	                sctp_lcd_spec_sample_sampling(cursor);
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
        }
    }
}

void SpecSample::exit(Sctp *sctp)
{
    free(check_result);
}

SctpState& SpecSample::getInstance()
{
	static SpecSample singleton;
	return singleton;
}