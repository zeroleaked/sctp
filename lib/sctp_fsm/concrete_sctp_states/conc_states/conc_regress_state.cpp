#include <esp_log.h>

#include "conc_regress_state.h"
#include "conc_table_state.h"
#include "sctp_lcd.h"

#define CURSOR_NULL 0
#define CURSOR_OK 1

#define SUBSTATE_FAIL 0
#define SUBSTATE_SUCCESS 1

static const char TAG[] = "conc_regress_state";

void ConcRegress::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;

	assert(sctp->curve.absorbance != NULL);
	assert(sctp->curve.concentration != NULL);


	if (sctp->curve.points < 2) {
		ESP_LOGI(TAG, "detected %d points, not enough points", sctp->curve.points);

		sctp_lcd_conc_regress_error(cursor);
		substate = SUBSTATE_FAIL;
	}
	else if ((sctp->curve.points == 2) && (sctp->curve.concentration[1] == 0)) { // this should never be true, nonstandard points don't count as curve.points
		ESP_LOGI(TAG, "detected 2 points, but 1 is not a standard. Not enough standards");
	}
	else {
		uint8_tstandards_length = sctp->curve.points;
		ESP_LOGI(TAG, "detected %d standard sample points", standards_length);
		assert(standards_length <= 15); // MAX_POINTS is 15
		assert(sctp->curve.points <= 15); // MAX_POINTS is 15
		bool interpolate;
		if (standards_length < 15) { // last row is not a standard point
			// check if we can interpolate		
			interpolate = (sctp->curve.absorbance[standards_length] != 0);
		}
		else {
			interpolate = false; // last row is also a standard. Not interpolating
		}

		float sum_absorbance = 0;
		float sum_absorbance_sq = 0;
		float sum_concentration = 0;
		float sum_product = 0;
		for (int i=0; i < standards_length; i++) {
			ESP_LOGI(TAG, "i=%d, (%f, %f)", i, sctp->curve.absorbance[i], sctp->curve.concentration[i]);
			sum_absorbance += sctp->curve.absorbance[i];
			sum_absorbance_sq += sctp->curve.absorbance[i] * sctp->curve.absorbance[i];
			sum_concentration += sctp->curve.concentration[i];
			sum_product += sctp->curve.absorbance[i] * sctp->curve.concentration[i];
		}
		float divider = standards_length * sum_absorbance_sq - sum_absorbance * sum_absorbance;
		assert(divider != 0);
		float gradient = (standards_length * sum_product - sum_absorbance * sum_concentration) / divider;
		float offset = (sum_concentration * sum_absorbance_sq - sum_absorbance * sum_product) / divider;
		ESP_LOGI(TAG, "m=%f, b=%f", gradient, offset);

		if (interpolate) {
			ESP_LOGI(TAG, "interpolating");
			sctp->curve.concentration[standards_length] = sctp->curve.absorbance[standards_length] * gradient + offset;
			sctp->curve.points++;
		}

		sctp_lcd_conc_regress(cursor, sctp->curve, interpolate);
		substate = SUBSTATE_SUCCESS;
	}
}

void ConcRegress::okay(Sctp* sctp)
{
	if (cursor == CURSOR_OK) {
		sctp->setState(ConcTable::getInstance());
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
			sctp_lcd_conc_regress(cursor, sctp->curve, interpolate);
			break;
		}
	}
}



SctpState& ConcRegress::getInstance()
{
	static ConcRegress singleton;
	return singleton;
}