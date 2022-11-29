#include <string.h>
#include <esp_log.h>

#include "sctp_lcd.h"
#include "conc_curves_state.h"
#include "conc_wavelength_state.h"
#include "conc_table_state.h"

#define SUBSTATE_WL 0
#define SUBSTATE_CURSOR 1

#define CURSOR_WL 0
#define CURSOR_NEXT 1
#define CURSOR_BACK 2

#define MAX_POINTS 10

static const char TAG[] = "conc_wavelength_state";

void ConcWavelength::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	wavelength = 550;
    substate = SUBSTATE_WL;
    cursor = CURSOR_WL;
	sctp_lcd_conc_wavelength(cursor, wavelength);
}

void ConcWavelength::okay(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        // Finish wavelength change
        substate = SUBSTATE_CURSOR;
    }
    else if (substate == SUBSTATE_CURSOR) {
        switch (cursor) {
            case CURSOR_WL: {
                substate = SUBSTATE_WL;
                break;
            }
            case CURSOR_NEXT: {
                sctp->curve.wavelength = wavelength;
                for (int j = 0; j < MAX_POINTS; j++)
                {
                    sctp->curve.concentration[j] = 0;
                    sctp->curve.absorbance[j] = -1;
                }
                // next state
                sctp->setState(ConcTable::getInstance()); 
                break;
            }
            case CURSOR_BACK: {
                // add curve cancels
                // free sctp->curve buffers
                if(sctp->curve.wavelength != 0) {
                    free(sctp->curve.filename);
                    ESP_LOGI(TAG, "free 1 done");
                    sctp->curve.filename = NULL;
                    free(sctp->curve.absorbance);
                    ESP_LOGI(TAG, "free 2 done");
                    sctp->curve.absorbance = NULL;
                    free(sctp->curve.concentration);
                    ESP_LOGI(TAG, "free 3 done");
                    sctp->curve.concentration = NULL;
                }
                sctp->setState(ConcCurves::getInstance());
                break;
            }
        }
    }
}

void ConcWavelength::arrowUp(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        wavelength = wavelength + 1;
        if (wavelength > 700) wavelength = 400;
        sctp_lcd_conc_wavelength_number(wavelength);
    }
    else if (substate == SUBSTATE_CURSOR) {
        sctp_lcd_conc_wavelength_clear(cursor);
        if (cursor == CURSOR_WL) cursor = CURSOR_NEXT;
        else cursor = CURSOR_WL;
        sctp_lcd_conc_wavelength(cursor, wavelength);
    }
}

void ConcWavelength::arrowDown(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        wavelength = wavelength - 1;
        if (wavelength < 400) wavelength = 700;
        sctp_lcd_conc_wavelength_number(wavelength);
    }
    else if (substate == SUBSTATE_CURSOR) {
        arrowUp(sctp);
    }
}

void ConcWavelength::arrowRight(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        arrowUp(sctp);
    }
    else if (substate == SUBSTATE_CURSOR) {
        sctp_lcd_conc_wavelength_clear(cursor);
        if (cursor == CURSOR_NEXT) cursor = CURSOR_BACK;
        else if (cursor == CURSOR_BACK) cursor = CURSOR_NEXT;
        sctp_lcd_conc_wavelength(cursor, wavelength);
    }
}

void ConcWavelength::arrowLeft(Sctp* sctp)
{
    if (substate == SUBSTATE_WL) {
        arrowDown(sctp);
    } else if (substate == SUBSTATE_CURSOR) {
        arrowRight(sctp);
    }
}

SctpState& ConcWavelength::getInstance()
{
	static ConcWavelength singleton;
	return singleton;
}