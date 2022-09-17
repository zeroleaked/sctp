#include "conc_table_state.h"
#include "sctp_lcd.h"

#define SUBSTATE_LOADING 0
#define SUBSTATE_CURSORING 1
#define SUBSTATE_CONCENTRATION 2

#define CURSOR_NULL 0
#define CURSOR

// static void loadConcFloats() {

// }

void ConcTable::enter(Sctp * sctp) {
	// sctp_lcd_clear();

	// cursor = CURSOR_NULL;
	// substate = SUBSTATE_LOADING;

	// sctp_lcd_conc_curves_opening(cursor);

    // xTaskCreatePinnedToCore(loadConcFloats, "curve floats load", 2048, sctp->curve, 4, NULL, 1);   
}

SctpState& ConcTable::getInstance()
{
	static ConcTable singleton;
	return singleton;
}