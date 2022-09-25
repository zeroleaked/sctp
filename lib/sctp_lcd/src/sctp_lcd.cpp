#include "sctp_lcd.h"

void sctp_lcd_start() {};
void sctp_lcd_clear() {};

void sctp_lcd_menu(uint8_t cursor) {};
void sctp_lcd_menu_clear(uint8_t cursor) {};

void sctp_lcd_spec_blank_waiting(uint8_t cursor) {};
void sctp_lcd_spec_blank_sampling(uint8_t cursor) {};
void sctp_lcd_spec_blank_clear(uint8_t cursor) {};

void sctp_lcd_spec_sample_waiting(uint8_t cursor) {};
void sctp_lcd_spec_sample_sampling(uint8_t cursor) {};
void sctp_lcd_spec_sample_clear(uint8_t cursor) {};

void sctp_lcd_spec_result(uint8_t cursor, float * wavelength, float * absorbance, uint16_t length) {};
void sctp_lcd_spec_result_cursor(uint8_t cursor) {};
void sctp_lcd_spec_result_clear(uint8_t cursor) {};

void sctp_lcd_spec_save_saving() {};
void sctp_lcd_spec_save_finish(char saved_name[20]) {};
void sctp_lcd_spec_save_finish_cursor(uint8_t cursor) {};

void sctp_lcd_conc_curves_opening(uint8_t cursor) {};
void sctp_lcd_conc_curves_list(uint8_t cursor, curve_t curves[6]) {};
void sctp_lcd_conc_curves_list_cursor(uint8_t cursor) {};
void sctp_lcd_conc_curves_list_clear(uint8_t cursor) {};
void sctp_lcd_conc_curves_loading_floats(uint8_t cursor) {};

void sctp_lcd_conc_wavelength(uint8_t cursor, uint16_t wavelength) {}
void sctp_lcd_conc_wavelength_clear(uint8_t cursor) {};
void sctp_lcd_conc_wavelength_number(uint16_t wavelength) {}

void sctp_lcd_conc_table_opening(uint8_t cursor) {};
void sctp_lcd_conc_table_cursor(uint8_t cursor, uint8_t row_offset, curve_t curve) {};
void sctp_lcd_conc_table_clear(uint8_t cursor, uint8_t row_offset, curve_t curve) {};
void sctp_lcd_conc_table_concentration(uint8_t cursor, float concentration) {};

void sctp_lcd_conc_blank_sampling(uint8_t cursor) {};
void sctp_lcd_conc_blank_waiting(uint8_t cursor) {};
void sctp_lcd_conc_blank_clear(uint8_t cursor) {};

void sctp_lcd_conc_sample_sampling(uint8_t cursor) {};
void sctp_lcd_conc_sample_waiting(uint8_t cursor) {};
void sctp_lcd_conc_sample_clear(uint8_t cursor) {};

void sctp_lcd_conc_regress(uint8_t cursor, curve_t curve, bool lastPointIsInterpolated, conc_regression_t * regress_line) {}
void sctp_lcd_conc_regress_error(uint8_t cursor) {};

void sctp_lcd_history_list(uint8_t cursor, history_t * history_list, uint8_t history_list_length) {};
void sctp_lcd_history_list_clear(uint8_t cursor) {};
