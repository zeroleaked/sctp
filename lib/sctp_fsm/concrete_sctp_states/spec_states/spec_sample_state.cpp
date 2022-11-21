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

static const float filter[] = {1.2443390815664472e-19,2.572174230657407e-05,5.1732513250363304e-05,7.790050974646357e-05,0.00010407257427968716,0.0001300657953100977,0.00015565992780143737,0.0001805910368076257,0.00020454674177511822,0.00022716340149051746,0.0002480255301362083,0.0002666676747497057,0.0002825789148584491,0.00029521006780315807,0.00030398360013604613,0.0003083061585551499,0.00030758354533940455,0.00030123787551318966,0.0002887265683766339,0.00026956274696528205,0.00024333654776089803,0.0002097367817524395,0.00016857233875109258,0.0001197926914713582,6.350683579465462e-05,-2.610812883998541e-19,-7.025253061829497e-05,-0.00014657509601235616,-0.00022808542379997466,-0.0003136929788859189,-0.00040210232471004736,-0.0004918218228212805,-0.0005811778863301849,-0.0006683348802895197,-0.0007513206305969416,-0.0008280573653101084,-0.0008963977712593939,-0.0009541657076705616,-0.000999200980422491,-0.001029407448867706,-0.0010428036151185563,-0.001037574736536301,-0.001012125408871538,-0.0009651314928684924,-0.000895590203628813,-0.0008028671517250576,-0.0006867391196220655,-0.0005474313775872244,-0.00038564839060986747,-0.00020259684201611683,6.188287743502073e-19,0.0002198974208418041,0.0004543346012934892,0.0007000506897933557,0.0009533119423480883,0.0012099500615259061,0.0014654116481695977,0.001714818453570692,0.0019530378901244567,0.0021747630294416966,0.0023746010931617093,0.0025471692279673375,0.002687196157173006,0.002789628121217678,0.0028497373626838267,0.0028632312819840185,0.0028263602910918156,0.0027360223276253585,0.002589861962628515,0.0023863620443084193,0.00212492586786452,0.001805947948742241,0.0014308716027725606,0.0010022317005645147,0.0005236811632908066,-1.0610289857644949e-18,-0.0005629140505489752,-0.0011580739218087436,-0.0017774510034098655,-0.00241204231931852,-0.0030519567755857187,-0.003686519811614543,-0.004304395448777122,-0.004893724397720762,-0.005442276567037069,-0.005937616017236872,-0.006367276131051292,-0.006718942529490492,-0.006980641057902322,-0.00714092800198714,-0.007189079574189783,-0.007115277639213163,-0.006910788625877992,-0.006568132602622665,-0.0060812395761217185,-0.005445590206396915,-0.004658338316060236,-0.0037184128036953515,-0.0026265968486877244,-0.0013855826130351346,1.4187764717148478e-18,0.0015235815856193588,0.003176679121465253,0.004948949714577489,0.006828271992216259,0.008800854476223382,0.010851369843508095,0.012963113549796259,0.015118184887363823,0.017297688167223376,0.019481951368899044,0.021650759292751554,0.023783597986418586,0.025859907003199217,0.027859335890196055,0.02976200120088882,0.03154874028274102,0.03320135810661508,0.03470286348133323,0.036037691132767107,0.03719190632043004,0.038153388912738236,0.038911994140999565,0.03945968759699234,0.039790652424109514,0.03990136707115838,0.039790652424109514,0.03945968759699234,0.038911994140999565,0.038153388912738236,0.03719190632043004,0.0360376911327671,0.03470286348133323,0.03320135810661508,0.03154874028274102,0.029762001200888816,0.027859335890196055,0.025859907003199213,0.023783597986418586,0.021650759292751554,0.019481951368899044,0.017297688167223373,0.015118184887363821,0.012963113549796257,0.010851369843508095,0.00880085447622338,0.006828271992216259,0.0049489497145774886,0.003176679121465253,0.0015235815856193585,1.4187764717148476e-18,-0.0013855826130351344,-0.0026265968486877244,-0.0037184128036953502,-0.004658338316060235,-0.005445590206396914,-0.006081239576121718,-0.006568132602622664,-0.006910788625877991,-0.007115277639213161,-0.007189079574189782,-0.007140928001987137,-0.006980641057902322,-0.00671894252949049,-0.00636727613105129,-0.005937616017236871,-0.005442276567037069,-0.004893724397720762,-0.004304395448777121,-0.0036865198116145426,-0.0030519567755857187,-0.00241204231931852,-0.0017774510034098653,-0.0011580739218087434,-0.0005629140505489749,-1.0610289857644947e-18,0.0005236811632908066,0.0010022317005645143,0.00143087160277256,0.001805947948742241,0.00212492586786452,0.0023863620443084184,0.002589861962628513,0.0027360223276253585,0.002826360291091815,0.0028632312819840177,0.002849737362683826,0.002789628121217678,0.0026871961571730057,0.0025471692279673366,0.002374601093161708,0.0021747630294416966,0.0019530378901244563,0.001714818453570691,0.0014654116481695968,0.0012099500615259052,0.000953311942348088,0.0007000506897933555,0.00045433460129348904,0.00021989742084180394,6.188287743502073e-19,-0.00020259684201611678,-0.0003856483906098673,-0.000547431377587224,-0.0006867391196220655,-0.0008028671517250576,-0.0008955902036288125,-0.0009651314928684914,-0.001012125408871538,-0.001037574736536301,-0.0010428036151185556,-0.0010294074488677047,-0.000999200980422491,-0.0009541657076705616,-0.0008963977712593933,-0.0008280573653101079,-0.0007513206305969405,-0.0006683348802895197,-0.0005811778863301844,-0.0004918218228212802,-0.0004021023247100468,-0.0003136929788859189,-0.00022808542379997452,-0.00014657509601235605,-7.025253061829487e-05,-2.610812883998541e-19,6.350683579465462e-05,0.00011979269147135815,0.00016857233875109233,0.0002097367817524395,0.00024333654776089803,0.00026956274696528183,0.00028872656837663363,0.00030123787551318966,0.00030758354533940455,0.00030830615855514975,0.00030398360013604597,0.00029521006780315774,0.0002825789148584491,0.00026666767474970554,0.0002480255301362081,0.00022716340149051733,0.00020454674177511822,0.0001805910368076256,0.0001556599278014373,0.0001300657953100976,0.00010407257427968716,7.790050974646357e-05,5.1732513250363304e-05,2.572174230657405e-05,1.2443390815664472e-19};

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