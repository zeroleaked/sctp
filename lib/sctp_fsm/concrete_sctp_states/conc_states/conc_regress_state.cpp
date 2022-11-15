#include <esp_log.h>

#include "conc_regress_state.h"
#include "conc_table_state.h"
#include "sctp_lcd.h"
#include "sctp_flash.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_FAIL 0
#define SUBSTATE_SUCCESS 1

#define MAX_POINTS 10

static const char TAG[] = "conc_regress_state";

void ConcRegress::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_OK;

	assert(sctp->curve.absorbance != NULL);
	assert(sctp->curve.concentration != NULL);

	regress_line = (conc_regression_t *) malloc (sizeof(conc_regression_t));

	if (sctp->curve.points < 2) {
		ESP_LOGI(TAG, "detected %d points, not enough points", sctp->curve.points);

		sctp_lcd_conc_regress_error(cursor);
		substate = SUBSTATE_FAIL;
	}
	else if ((sctp->curve.points == 2) && (sctp->curve.concentration[1] == 0)) { // this should never be true, nonstandard points don't count as curve.points
		ESP_LOGI(TAG, "detected 2 points, but 1 is not a standard. Not enough standards");
	}
	else {
		uint8_t standards_length;
		if (sctp->curve.concentration[sctp->curve.points] == 0)
			standards_length = sctp->curve.points - 1;
		else
			standards_length = sctp->curve.points;
		ESP_LOGI(TAG, "detected %d standard sample points", standards_length);
		assert(standards_length <= MAX_POINTS);	  // MAX_POINTS is 10
		assert(sctp->curve.points <= MAX_POINTS); // MAX_POINTS is 10
		if (standards_length < MAX_POINTS)
		{ // last row is not a standard point
			// check if we can interpolate
			interpolate = (sctp->curve.absorbance[standards_length] != 0);
			sctp->lastPointIsInterpolated = true;
		}
		else {
			interpolate = false; // last row is also a standard. Not interpolating
		}

		float sum_absorbance = 0;
		float sum_concentration = 0;
		float sum_absorbance_sq = 0;
		float sum_product = 0;
		for (int i=0; i < standards_length; i++) {
			ESP_LOGI(TAG, "i=%d, (%f, %f)", i, sctp->curve.absorbance[i], sctp->curve.concentration[i]);
			sum_absorbance += sctp->curve.absorbance[i];
			sum_concentration += sctp->curve.concentration[i];
			sum_absorbance_sq += sctp->curve.absorbance[i] * sctp->curve.absorbance[i];
			sum_product += sctp->curve.absorbance[i] * sctp->curve.concentration[i];
		}
		float divider = standards_length * sum_absorbance_sq - sum_absorbance * sum_absorbance;
		assert(divider != 0);
		regress_line->gradient = (standards_length * sum_product - sum_concentration * sum_absorbance) / divider;
		regress_line->offset = (sum_concentration * sum_absorbance_sq - sum_absorbance * sum_product) / divider;
		ESP_LOGI(TAG, "m=%f, b=%f", regress_line->gradient, regress_line->offset);

		if (interpolate) {
			ESP_LOGI(TAG, "interpolating");
			sctp->curve.concentration[standards_length] = sctp->curve.absorbance[standards_length] * regress_line->gradient + regress_line->offset;
			// sctp->curve.points++;
		}

		sctp_lcd_conc_regress(cursor, sctp->curve, interpolate, regress_line);
		substate = SUBSTATE_SUCCESS;
	}
}

void ConcRegress::okay(Sctp* sctp)
{
	switch (substate) {
		case SUBSTATE_FAIL: {
			if (cursor == CURSOR_OK) {
				sctp->setState(ConcTable::getInstance());
			}
			break;
		}
		case SUBSTATE_SUCCESS: {
			if (cursor == CURSOR_OK) {
				sctp->setState(ConcTable::getInstance());
			}
			break;
		}
	}
}

void ConcRegress::arrowDown(Sctp* sctp)
{
	cursor = CURSOR_OK;
	switch (substate) {
		case SUBSTATE_FAIL: {
			sctp_lcd_conc_regress_error(cursor);
			break;
		}
		case SUBSTATE_SUCCESS: {
			sctp_lcd_conc_regress(cursor, sctp->curve, interpolate, regress_line);
			break;
		}
	}
}

void ConcRegress::exit(Sctp* sctp)
{
	if (regress_line != NULL) {
		free(regress_line);
		regress_line = NULL;
	} 
}

SctpState& ConcRegress::getInstance()
{
	static ConcRegress singleton;
	return singleton;
}