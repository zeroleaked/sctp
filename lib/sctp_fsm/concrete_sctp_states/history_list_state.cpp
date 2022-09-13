#include <esp_log.h>

#include "history_list_state.h"
#include "sctp_lcd.h"
#include "menu_state.h"

#define CURSOR_NULL 11
#define CURSOR_BACK 12

#define LOADING_HISTORY 11


static const char TAG[] = "history_list_state";

// dummies
void sctp_flash_history(Sctp * sctp) {
	sctp->history_list_length = 5;
	sctp->history_list = (history_t *) malloc(sctp->history_list_length * sizeof(history_t));
	if (sctp->history_list != NULL) {
		ESP_LOGI(TAG, "history list allocated");
	}
	else {
		ESP_LOGE(TAG, "failed to allocate history list");
		sctp->history_list_length = 0;
		return;
	}
	for (int i=0; i<sctp->history_list_length; i++) {
		sctp->history_list[i].id = i;
		sctp->history_list[i].measurement_mode = MEASUREMENT_MODE_CONCENTRATION;
		sctp->history_list[i].wavelength = 400 + i*10;
		sctp->history_list[i].absorbance = 0.01 * (i+3);
		sctp->history_list[i].concentration = 0.001 * (i+3);
		sctp->history_list[i].curve_id = 0;
	}	
};

void HistoryList::enter(Sctp* sctp)
{
	sctp_lcd_clear();
	cursor = CURSOR_NULL;
	page = 0;
	page_length = 0;
	next_page = false;
	sctp_lcd_history_list(cursor, NULL, LOADING_HISTORY);
	sctp_flash_history(sctp);
}

// update lcd with loaded history list from flash
void HistoryList::updateLcd(Sctp* sctp)
{
	sctp_lcd_clear();
	page = 0;
	if (sctp->history_list_length >= 10) page_length = 10;
	else page_length = sctp->history_list_length;
	sctp_lcd_history_list(cursor, sctp->history_list, page_length);
}

void HistoryList::okay(Sctp* sctp)
{
	if ( cursor < CURSOR_BACK ) {
		sctp->history_index = page*10 + cursor;

		// copy selected history
		sctp->history = sctp->history_list[sctp->history_index];

		if ( sctp->history.measurement_mode == MEASUREMENT_MODE_CONCENTRATION ) {
			// sctp->setState(HistoryConc::getInstance());
		}
		else {
			// sctp->setState(HistorySpec::getInstance());
		}
	}
	else switch (cursor) {
		case CURSOR_BACK: {
			sctp->setState(Menu::getInstance());
			break;
		}
		case CURSOR_NULL: { break; } // do nothing
	}
}

void HistoryList::arrowUp(Sctp* sctp)
{
	sctp_lcd_history_list_clear(cursor);

	if (cursor < CURSOR_NULL) {
		if (cursor == 0) {
			cursor = CURSOR_BACK;
		}
		else cursor --;
	}
	else switch (cursor) {
		case CURSOR_NULL: {
			cursor = CURSOR_BACK;
			break;
		}
		case CURSOR_BACK: {
			if (page_length > 0) {
				cursor = page_length - 1;
			}
			break;
		}
	}

	sctp_lcd_history_list(cursor, &(sctp->history_list[page*10]), page_length);
}

void HistoryList::arrowDown(Sctp* sctp)
{
	sctp_lcd_history_list_clear(cursor);

	if (cursor < CURSOR_NULL) {
		if (cursor == 4) {
			cursor = CURSOR_BACK;
		}
		else cursor ++;
	}
	else switch (cursor) {
		case CURSOR_NULL: {
			cursor = CURSOR_BACK;
			break;
		}
		case CURSOR_BACK: {
			if (page_length > 0) {
				cursor = 0;
			}
			break;
		}
	}
	sctp_lcd_history_list(cursor, &(sctp->history_list[page*10]), page_length);
}

void HistoryList::arrowRight(Sctp* sctp)
{
	sctp_lcd_history_list_clear(cursor);

	if (cursor < CURSOR_NULL) {
		if (cursor > 4) {
			// check if there is more page
			if (sctp->history_list_length > page*10) {
				page++;
				cursor = cursor - 5;
			}
			else {} // do nothing
		}
		else cursor += 5;
	}
	else switch (cursor) {
		case CURSOR_NULL: {
			cursor = CURSOR_BACK;
			break;
		}
		case CURSOR_BACK: {
			// do nothing
			break;
		}
	}

	sctp_lcd_history_list(cursor, &(sctp->history_list[page*10]), page_length);
}

void HistoryList::arrowLeft(Sctp* sctp)
{
	sctp_lcd_history_list_clear(cursor);

	if (cursor < CURSOR_NULL) {
		if (cursor < 5) {
			if (page != 0) {
				page--;
				cursor = cursor + 5;
			}
			else {} // do nothing
		}
		else cursor = cursor - 5;
	}
	else switch (cursor) {
		case CURSOR_NULL: {
			cursor = CURSOR_BACK;
			break;
		}
		case CURSOR_BACK: {
			// do nothing
			break;
		}
	}

	sctp_lcd_history_list(cursor, &(sctp->history_list[page*10]), page_length);
}

void HistoryList::exit(Sctp* sctp)
{
	free(sctp->history_list);
	sctp->history_list = NULL;
	sctp->history_list_length = 0;
}

SctpState& HistoryList::getInstance()
{
	static HistoryList singleton;
	return singleton;
}