#include "sctp_lcd.h"
#include "math.h"
#include <esp_log.h>

static const char TAG[] = "lcd";

#define TFT_TOSCA 0x5D35
#define TFT_MUSTARD 0xD461

static LGFX display;
static LGFX_Sprite sprite(&display);

void sctp_lcd_clear() {
  ESP_LOGI(TAG, "pt4");
  display.fillScreen(TFT_WHITE);
  ESP_LOGI(TAG, "pt5");
  display.drawFastVLine(479, 1, 315, TFT_BLUE);
  ESP_LOGI(TAG, "pt6");
  display.drawFastHLine(0, 319, 480, TFT_RED);
  ESP_LOGI(TAG, "pt7");
};

void sctp_lcd_start() {
    display.init();
    display.fillScreen(TFT_WHITE);
    display.setFont(&fonts::FreeSansBold24pt7b);
    display.setTextColor(TFT_TOSCA);
    display.setTextSize(1);
    // half width - num int * int width in pixels
    display.setCursor(185, 75);
    display.println("SCTP");
    display.setFont(&fonts::FreeSansBold12pt7b);
    display.setTextColor(TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(85, 125);
    display.println("Visible Spectrophotometer");
    display.setFont(&fonts::FreeSansBold9pt7b);
    display.setTextColor(TFT_MUSTARD);
    display.setTextSize(1);
    display.setCursor(140, 225);
    display.println("Press any button to start");
}

void sctp_lcd_menu_clear(uint8_t cursor)
{
    switch(cursor) {
        case 0: {
      display.fillRoundRect(120, 60, 245, 40, 10, TFT_WHITE);
      break;
    }
    case 1:{
      display.fillRoundRect(120, 110, 245, 40, 10, TFT_WHITE);
      break;
    }
    case 2:{
      display.fillRoundRect(120, 160, 245, 40, 10, TFT_WHITE);
      break;
    }
    case 3:{
      display.fillRoundRect(120, 210, 245, 40, 10, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_menu(uint8_t cursor)
{
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
    switch(cursor) {
        case 0: {
      display.fillRoundRect(120, 60, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 1:{
      display.fillRoundRect(120, 110, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 2:{
      display.fillRoundRect(120, 160, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 3:{
      display.fillRoundRect(120, 210, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
    }
  display.setCursor(165, 75);
  display.println("SPECTRUM MODE");
  display.drawRoundRect(120, 60, 245, 40, 10, TFT_BLACK);
  display.setCursor(145, 125);
  display.println("QUANTIZATION MODE");
  display.drawRoundRect(120, 110, 245, 40, 10, TFT_BLACK);
  display.setCursor(205, 175);
  display.println("LIBRARY");
  display.drawRoundRect(120, 160, 245, 40, 10, TFT_BLACK);
  display.setCursor(198, 225);
  display.println("SETTINGS");
  display.drawRoundRect(120, 210, 245, 40, 10, TFT_BLACK);
}

void sctp_lcd_blank_clear(uint8_t cursor)
{
  switch (cursor)
  {
    case 0:
    {
      display.fillRoundRect(340, 265, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 1:
    {
      display.fillRoundRect(180, 265, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 2:
    {
      display.fillRoundRect(20, 265, 120, 40, 10, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_blank_waiting(uint8_t cursor, uint16_t result)
{
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  switch (cursor)
  {
    case 0:
    {
      display.fillRoundRect(340, 265, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 1:
    {
      display.fillRoundRect(180, 265, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 2:
    {
      display.fillRoundRect(20, 265, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
  }
  display.setTextSize(1);
  display.setCursor(170, 125);
  display.println("Place reference");

  if (result != 0)
  {
    display.setTextColor(TFT_MUSTARD);
    display.fillRect(170, 170, 150, 80, TFT_WHITE);
    display.setCursor(180, 175);
    char text[20];
    sprintf(text, "Intensity: %d", result);
    display.println(text);
  }

  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(42, 277);
  display.println("CANCEL");
  display.drawRoundRect(20, 265, 120, 40, 10, TFT_BLACK);
  display.setCursor(208, 277);
  display.println("CHECK");
  display.drawRoundRect(180, 265, 120, 40, 10, TFT_BLACK);
  display.setCursor(375, 277);
  display.println("NEXT");
  display.drawRoundRect(340, 265, 120, 40, 10, TFT_BLACK);
}

void sctp_lcd_blank_sampling(uint8_t cursor)
{
  display.fillRect(85, 170, 350, 80, TFT_WHITE);
  display.setTextColor(TFT_MUSTARD);
  display.setCursor(140, 175);
  display.println("Measuring Reference...");
}

void sctp_lcd_sample_clear(uint8_t cursor)
{
  switch (cursor)
  {
    case 0:
    {
      display.fillRoundRect(340, 265, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 1:
    {
      display.fillRoundRect(180, 265, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 2:
    {
      display.fillRoundRect(20, 265, 120, 40, 10, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_sample_waiting(uint8_t cursor, uint16_t result)
{
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  switch (cursor)
  {
    case 0:
    {
      display.fillRoundRect(340, 265, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 1:
    {
      display.fillRoundRect(180, 265, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 2:
    {
      display.fillRoundRect(20, 265, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
  }
  display.setTextSize(1);
  display.setCursor(180, 125);
  display.println("Place sample");

  if (result != 0)
  {
    display.setTextColor(TFT_MUSTARD);
    display.fillRect(170, 170, 150, 80, TFT_WHITE);
    display.setCursor(180, 175);
    char text[20];
    sprintf(text, "Intensity: %d", result);
    display.println(text);
  }

  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(42, 277);
  display.println("CANCEL");
  display.drawRoundRect(20, 265, 120, 40, 10, TFT_BLACK);
  display.setCursor(208, 277);
  display.println("CHECK");
  display.drawRoundRect(180, 265, 120, 40, 10, TFT_BLACK);
  display.setCursor(375, 277);
  display.println("NEXT");
  display.drawRoundRect(340, 265, 120, 40, 10, TFT_BLACK);
}

void sctp_lcd_sample_sampling(uint8_t cursor)
{
  display.fillRect(95, 170, 350, 80, TFT_WHITE);
  display.setTextColor(TFT_MUSTARD);
  display.setCursor(150, 175);
  display.println("Measuring Sample...");
}

void sctp_lcd_save_saving()
{
  display.setTextColor(TFT_MUSTARD);
  display.setTextSize(1);
  display.setCursor(215, 125);
  display.println("Saving...");
}

void sctp_lcd_save_finish(char saved_name[25]){
  display.fillRect(210, 120, 90, 30, TFT_WHITE);
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(140, 125);
  char text[] = "Saved as ...............";
  sprintf(text, "Saved as %s", saved_name);
  display.println(text);
  display.fillRoundRect(180, 160, 120, 40, 10, TFT_LIGHTGREY);
  display.setTextColor(TFT_BLACK);
  display.setCursor(230, 175);
  display.println("OK");
  display.drawRoundRect(180, 160, 120, 40, 10, TFT_BLACK);
}

void sctp_lcd_save_finish_cursor(uint8_t cursor){
  display.fillRoundRect(180, 160, 120, 40, 10, TFT_LIGHTGREY);
  display.setTextSize(1);
  display.setTextColor(TFT_BLACK);
  display.setCursor(230, 175);
  display.println("OK");
  display.drawRoundRect(180, 160, 120, 40, 10, TFT_BLACK);
}

void sctp_lcd_spec_save_saving(){
  sctp_lcd_save_saving();
}

void sctp_lcd_spec_save_finish(char saved_name[255]){
  sctp_lcd_save_finish(saved_name);
}

void sctp_lcd_spec_save_finish_cursor(uint8_t cursor){
  sctp_lcd_save_finish_cursor(cursor);
}

void sctp_lcd_spec_blank_sampling(uint8_t cursor){
  sctp_lcd_blank_sampling(cursor);
}

void sctp_lcd_spec_blank_waiting(uint8_t cursor, uint16_t result)
{
  sctp_lcd_blank_waiting(cursor, result);
}

void sctp_lcd_spec_blank_clear(uint8_t cursor){
  sctp_lcd_blank_clear(cursor);
}

void sctp_lcd_spec_sample_sampling(uint8_t cursor){
  sctp_lcd_sample_sampling(cursor);
}

void sctp_lcd_spec_sample_waiting(uint8_t cursor, uint16_t result)
{
  sctp_lcd_sample_waiting(cursor, result);
}

void sctp_lcd_spec_sample_clear(uint8_t cursor){
  sctp_lcd_sample_clear(cursor);
}

void sctp_lcd_spec_result_clear(uint8_t cursor)
{
  switch(cursor){
    case 0:
    {
      display.fillRoundRect(365, 45, 100, 35, 5, TFT_WHITE);
      break;
    }
    case 1:{
      display.fillRoundRect(365, 95, 100, 35, 5, TFT_WHITE);
      break;
    }
    case 2:{
      display.fillRoundRect(365, 145, 100, 35, 5, TFT_WHITE);
      break;
    }
    case 3:{
      display.fillRoundRect(365, 195, 100, 35, 5, TFT_WHITE);
      break;
    }
    case 4:{
      display.fillRoundRect(365, 245, 100, 35, 5, TFT_WHITE);
      break;
    }
    default:{
      display.fillRoundRect(365, 45, 100, 35, 5, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_spec_result(uint8_t cursor, float * wavelength, float * absorbance, uint16_t length)
{
  int i = 0;
  float peak_abs = absorbance[0];
  float peak_wl= wavelength[0];
  
  for(i=0;i<length;i++){
    if(i==0){
      if(absorbance[i] > peak_abs){
        peak_abs = absorbance[i];
        peak_wl = wavelength[i];
      }
    }
    else{
      if(absorbance[i] > peak_abs){
        peak_abs = absorbance[i];
        peak_wl = wavelength[i];
      }
    }
  }
  peak_wl = round(peak_wl);
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(35, 95);
  display.println("PEAK WAVELENGTH: ");
  display.setTextColor(TFT_TOSCA);
  display.setCursor(250, 95);
  display.println((int)peak_wl);
  display.setTextColor(TFT_BLACK);
  display.setCursor(300, 95);
  display.println("nm");
  display.setCursor(35, 145);
  display.println("PEAK ABSORBANCE: ");
  display.setTextColor(TFT_TOSCA);
  display.setCursor(250, 145);
  display.println(peak_abs);

  switch(cursor){
    case 0:{
      display.fillRoundRect(365, 45, 100, 35, 5, TFT_LIGHTGREY);
      break;
    }    
    case 1:{
      display.fillRoundRect(365, 95, 100, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 2:{
      display.fillRoundRect(365, 145, 100, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 3:{
      display.fillRoundRect(365, 195, 100, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 4:{
      display.fillRoundRect(365, 245, 100, 35, 5, TFT_LIGHTGREY);
      break;
    }
    default:{
      display.fillRoundRect(365, 45, 100, 35, 5, TFT_LIGHTGREY);
      break;
    }
  }
  display.setTextColor(TFT_BLACK);
  display.setTextSize(0.75);
  display.drawRoundRect(365, 45, 100, 35, 5, TFT_BLACK);
  display.setCursor(380, 58);
  display.println("SPECTRUM");
  display.drawRoundRect(365, 95, 100, 35, 5, TFT_BLACK);
  display.setCursor(400, 108);
  display.println("SAVE");
  display.drawRoundRect(365, 145, 100, 35, 5, TFT_BLACK);
  display.setCursor(380, 158);
  display.println("RESAMPLE");
  display.drawRoundRect(365, 195, 100, 35, 5, TFT_BLACK);
  display.setCursor(372, 208);
  display.println("CHANGE REF");
  display.drawRoundRect(365, 245, 100, 35, 5, TFT_BLACK);
  display.setCursor(377, 258);
  display.println("MAIN MENU");
}

void sctp_lcd_spec_result_full(float * wavelength, float * absorbance, uint16_t length) {
  display.setTextColor(TFT_TOSCA);
  display.setTextSize(0.75);
  display.setCursor(5, 8);
  display.println("Absorbance (x0.001) VS. Wavelength (nm)");
  display.drawRect(35, 27, 363, 271, TFT_BLACK);

  float a_max = absorbance[0];
  float a_min = 0;
  float wl_min = wavelength[0];
  float wl_max = wavelength[length-1];
  if (absorbance[0] == 0)
  {
    absorbance[0] = 0;
  }
  for (int i = 1; i < length; i++)
  {
    if (absorbance[i] >= a_max)
      a_max = absorbance[i];
    if (absorbance[i] < 0)
    {
      absorbance[i] = 0;
    }
  }
  if (a_max <= 0.01)
    a_max = 0.01;
  else if (a_max <= 0.02)
    a_max = 0.02;
  else if (a_max <= 0.04)
    a_max = 0.04;
  else if(a_max <= 0.1)
    a_max = 0.1;
  else if (a_max <= 0.2)
    a_max = 0.2;
  else
    a_max = 0.4;

  ESP_LOGI(TAG, "LCD check 1");
  display.setTextColor(TFT_TOSCA);
  display.setTextSize(1);

  for(int i=0; i<5; i++) {
    char a[] = "X.X";
    sprintf(a, "%.1f", (double)a_max - a_max/4*i);
    display.setCursor(5, 30 + 65*i);
    display.println(a);
    display.setCursor(22 + 90*i, 302);
    display.println((int)round(wl_min + (wl_max-wl_min)/4*i));
  }
  ESP_LOGI(TAG, "LCD check 2");
  int i = 0;
  int i_prev;
  int x_px;
  int y_px;
  int x_next;
  int y_prev;

  if (length >= 360)
  {
    for (i = 0; i < length; i++)
    {
      x_px = (wavelength[i] - wl_min) / (wl_max - wl_min) * 360 + 36;
      x_next = (wavelength[i+1] - wl_min) / (wl_max - wl_min) * 360 + 36;
      y_px = 297 - (absorbance[i] - a_min) / (a_max - a_min) * 270;
      display.fillRect(x_px - 1, y_px - 1, 3, 3, TFT_TOSCA);
      if (i == 0)
        y_prev = y_px;
      int dy = abs(y_px - y_prev);
      if (dy > 3 && dy < 6)
      {
        if (y_prev < y_px)
          display.fillRect(x_px - 1, (y_prev + dy / 2) - 1, 3, 3, TFT_TOSCA);
        else
          display.fillRect(x_px - 1, (y_prev - dy / 2) - 1, 3, 3, TFT_TOSCA);
      }
      else if (dy >= 6 && dy < 12)
      {
        if (y_prev < y_px)
        {
          display.fillRect(x_px - 1, (y_prev + dy / 3) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 2 / 3) - 1, 3, 3, TFT_TOSCA);
        }
        else
        {
          display.fillRect(x_px - 1, (y_prev - dy / 3) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy * 2 / 3) - 1, 3, 3, TFT_TOSCA);
        }
      }
      else if (dy >= 12 && dy <21)
      {
        if (y_prev < y_px)
        {
          display.fillRect(x_px - 2, (y_prev + dy / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 2 / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 3 / 4) - 1, 3, 3, TFT_TOSCA);
        }
        else
        {
          display.fillRect(x_px - 2, (y_prev - dy * 3 / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy * 2 / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy / 4) - 1, 3, 3, TFT_TOSCA);
        }
      }
      else if (dy >= 21)
      {
        if (y_prev < y_px)
        {
          display.fillRect(x_px - 2, (y_prev + dy / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 2 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 3 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 4 / 5) - 1, 3, 3, TFT_TOSCA);
        }
        else
        {
          display.fillRect(x_px - 2, (y_prev - dy * 4 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 2, (y_prev - dy * 3 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy * 2 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy / 5) - 1, 3, 3, TFT_TOSCA);
        }
      }
      y_prev = y_px;
    }
    if(x_px == x_next) i = i + 2;
  }
  else
  {
    y_prev = 297 - (absorbance[0] - a_min) / (a_max - a_min) * 270;
    ESP_LOGI(TAG, "LCD check 3");
    for (int j = 0; j < 360; j++)
    {
      i = round(j * length / 360);
      i_prev = round((j-1) / length * 360);
      if(i == 0) i_prev = i;
      x_px = (wavelength[i] - wl_min) / (wl_max - wl_min) * 360 + 36;
      // x_next = (wavelength[i + 1] - wl_min) / (wl_max - wl_min) * 360 + 36;
      if(i == i_prev) {
        y_px = 297 - ((absorbance[i]+absorbance[i+1])/2 - a_min) / (a_max - a_min) * 270;
      } else {
        y_px = 297 - (absorbance[i] - a_min) / (a_max - a_min) * 270;
      }
      // ESP_LOGI(TAG, "i=%d, x=%d, y=%d", i, x_px, y_px);
      display.fillRect(x_px - 1, y_px - 1, 3, 3, TFT_TOSCA);
      int dy = abs(y_px - y_prev);
      if (dy > 3 && dy < 6)
      {
        if (y_prev < y_px)
          display.fillRect(x_px - 1, (y_prev + dy / 2) - 1, 3, 3, TFT_TOSCA);
        else
          display.fillRect(x_px - 1, (y_prev - dy / 2) - 1, 3, 3, TFT_TOSCA);
      }
      else if (dy >= 6 && dy < 12)
      {
        if (y_prev < y_px)
        {
          display.fillRect(x_px - 1, (y_prev + dy / 3) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 2 / 3) - 1, 3, 3, TFT_TOSCA);
        }
        else
        {
          display.fillRect(x_px - 1, (y_prev - dy / 3) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy * 2 / 3) - 1, 3, 3, TFT_TOSCA);
        }
      }
      else if (dy >= 12 && dy < 21)
      {
        if (y_prev < y_px)
        {
          display.fillRect(x_px - 2, (y_prev + dy / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 2 / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 3 / 4) - 1, 3, 3, TFT_TOSCA);
        }
        else
        {
          display.fillRect(x_px - 2, (y_prev - dy * 3 / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy * 2 / 4) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy / 4) - 1, 3, 3, TFT_TOSCA);
        }
      }
      else if (dy >= 21)
      {
        if (y_prev < y_px)
        {
          display.fillRect(x_px - 2, (y_prev + dy / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 2 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 3 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev + dy * 4 / 5) - 1, 3, 3, TFT_TOSCA);
        }
        else
        {
          display.fillRect(x_px - 2, (y_prev - dy * 4 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 2, (y_prev - dy * 3 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy * 2 / 5) - 1, 3, 3, TFT_TOSCA);
          display.fillRect(x_px - 1, (y_prev - dy / 5) - 1, 3, 3, TFT_TOSCA);
        }
      }
      y_prev = y_px;
    }
  }
  // display.setTextColor(TFT_TOSCA);
  // display.setTextSize(0.75);
  // display.setCursor(404, 260);
  // display.println("Wavelength");
  // display.setCursor(404, 275);
  // display.println("(nm)");
}

void sctp_lcd_conc_curves_loading_floats(uint8_t cursor){}

void sctp_lcd_conc_curves_list_clear(uint8_t cursor)
{
  switch(cursor) {
    case 0: {
      display.fillRoundRect(120, 18, 200, 35, 5, TFT_WHITE);
      break;
    }
    case 1: {
      display.fillRoundRect(120, 68, 200, 35, 5, TFT_WHITE);
      break;
    }
    case 2: {
      display.fillRoundRect(120, 118, 200, 35, 5, TFT_WHITE);
      break;
    }
    case 3: {
      display.fillRoundRect(120, 168, 200, 35, 5, TFT_WHITE);
      break;
    }
    case 4: {
      display.fillRoundRect(120, 218, 200, 35, 5, TFT_WHITE);
      break;
    }
    case 5: {
      display.fillRoundRect(120, 268, 200, 35, 5, TFT_WHITE);
      break;
    }
    case 6: {
      display.fillRoundRect(330, 18, 35, 35, 5, TFT_WHITE);
      break;
    }
    case 7: {
      display.fillRoundRect(330, 68, 35, 35, 5, TFT_WHITE);
      break;
    }
    case 8: {
      display.fillRoundRect(330, 118, 35, 35, 5, TFT_WHITE);
      break;
    }
    case 9: {
      display.fillRoundRect(330, 168, 35, 35, 5, TFT_WHITE);
      break;
    }
    case 10: {
      display.fillRoundRect(330, 218, 35, 35, 5, TFT_WHITE);
      break;
    }
    case 11: {
      display.fillRoundRect(330, 268, 35, 35, 5, TFT_WHITE);
      break;
    }
    case 12: {
      display.fillRoundRect(25, 268, 80, 35, 5, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_conc_curves_list(uint8_t cursor, curve_t curves[6]){
  int x = 120;
  int y = 18;
  int x_text =  x + 20;
  int y_text = y + 7;

  switch(cursor) {
    case 0: {
      display.fillRoundRect(120, 18, 200, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 1: {
      display.fillRoundRect(120, 68, 200, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 2: {
      display.fillRoundRect(120, 118, 200, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 3: {
      display.fillRoundRect(120, 168, 200, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 4: {
      display.fillRoundRect(120, 218, 200, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 5: {
      display.fillRoundRect(120, 268, 200, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 6: {
      display.fillRoundRect(330, 18, 35, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 7: {
      display.fillRoundRect(330, 68, 35, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 8: {
      display.fillRoundRect(330, 118, 35, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 9: {
      display.fillRoundRect(330, 168, 35, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 10: {
      display.fillRoundRect(330, 218, 35, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 11: {
      display.fillRoundRect(330, 268, 35, 35, 5, TFT_LIGHTGREY);
      break;
    }
    case 12: {
      display.fillRoundRect(25, 268, 80, 35, 5, TFT_LIGHTGREY);
      break;
    }
  }

  for(int i=0;i<6;i++){
    if(curves[i].wavelength != 0){
      display.setTextColor(TFT_BLACK);
      display.setTextSize(1);
      display.drawRoundRect(x, y, 200, 35, 5, TFT_BLACK);
      display.setCursor(x_text, y_text);
      char curve_name[25];
      sprintf(curve_name, "curve %d %dnm", curves[i].id+1, curves[i].wavelength);
      display.println(curve_name);

      display.setTextColor(TFT_RED);
      display.setTextSize(1 );
      display.drawRoundRect(x+210, y, 35, 35, 5, TFT_BLACK);
      display.setCursor(x+223, y_text+3);
      display.println("X");
    }
    else{
      display.setTextColor(TFT_TOSCA);
      display.setTextSize(1.25);
      display.drawRoundRect(x, y, 200, 35, 5, TFT_BLACK);
      display.fillRect(x+99, y+5, 3, 25, TFT_TOSCA);
      display.fillRect(x+87, y+17, 25, 3, TFT_TOSCA);
      // display.setCursor(215, y_text);
      // display.println("+");
    }
    y += 50;
    y_text = y + 7;
  }

  display.drawRoundRect(25, 268, 80, 35, 5, TFT_BLACK);
  display.setTextColor(TFT_BLACK);
  display.setTextSize(0.75);
  display.setCursor(48, 281);
  display.println("BACK");
}

void sctp_lcd_conc_curves_list_cursor(uint8_t cursor){}

void sctp_lcd_conc_wavelength(uint8_t cursor, uint16_t wavelength) {
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1.25);
  display.setCursor(60, 70);
  display.println("Set wavelength for measurement");
  display.setCursor(60, 120);
  display.println("WAVELENGTH:");

  switch(cursor) {
    case 0:{
      display.fillRect(235, 105, 90, 45, TFT_LIGHTGREY);
      break;
    }
    case 1: {
      display.fillRoundRect(245, 160, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 2: {
      display.fillRoundRect(120, 160, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
  }

  display.setCursor(240, 120);
  char wl[10] = "XXX nm";
  sprintf(wl, "%03d nm", wavelength);
  display.println(wl);

  display.fillTriangle(400, 135, 410, 120, 420, 135, TFT_BLACK);
  display.fillTriangle(430, 120, 440, 135, 450, 120, TFT_BLACK);

  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  
  display.setCursor(155, 175);
  display.println("BACK");
  display.drawRoundRect(120, 160, 120, 40, 10, TFT_BLACK);
  display.setCursor(280, 175);
  display.println("NEXT");
  display.drawRoundRect(245, 160, 120, 40, 10, TFT_BLACK);
}

void sctp_lcd_conc_wavelength_number(uint16_t wavelength) {
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1.25);
  display.fillRect(235, 105, 90, 45, TFT_LIGHTGREY);
  display.setCursor(240, 120);
  char wl[10] = "XXX nm";
  sprintf(wl, "%03d nm", wavelength);
  display.println(wl);
}

void sctp_lcd_conc_wavelength_clear(uint8_t cursor) {
  switch(cursor) {
    case 0: {
      display.fillRect(235, 105, 90, 45, TFT_WHITE);
      break;
    }
    case 1: {
      display.fillRoundRect(245, 160, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 2: {
      display.fillRoundRect(120, 160, 120, 40, 10, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_conc_table_opening(uint8_t cursor){
  display.setTextColor(TFT_MUSTARD);
  display.setCursor(120, 250);
  display.println("Loading Calibration Curve...");
}

void sctp_lcd_conc_table_cursor(uint8_t cursor, uint8_t row_offset, curve_t curve, bool lastPointIsInterpolated)
{
  display.setTextColor(TFT_TOSCA);
  display.setTextSize(1.5);
  display.setCursor(200, 35);
  display.println("Conc");
  display.setCursor(360, 35);
  display.println("A");

  int x0 = 90;
  int x1 = 200;
  int x2 = 340;
  int y = 80;
  char a[] = "X.XX";
  char c[] = "X.XXX";
  display.setTextColor(TFT_BLACK);

  switch(cursor) {
    case 0: {
      display.fillRect(195, 75, 75, 35, TFT_LIGHTGREY);
      break;
    }
    case 1: {
      display.fillRect(195, 115, 75, 35, TFT_LIGHTGREY);
      break;
    }
     case 2: {
      display.fillRect(195, 155, 75, 35, TFT_LIGHTGREY);
      break;
    }
    case 3: {
      display.fillRect(195, 195, 75, 35, TFT_LIGHTGREY);
      break;
    }
    case 4: {
      display.fillRect(335, 75, 75, 35, TFT_LIGHTGREY);
      break;
    }
    case 5: {
      display.fillRect(335, 115, 75, 35, TFT_LIGHTGREY);
      break;
    }
     case 6: {
      display.fillRect(335, 155, 75, 35, TFT_LIGHTGREY);
      break;
    }
    case 7: {
      display.fillRect(335, 195, 75, 35, TFT_LIGHTGREY);
      break;
    }
    case 8: {
      display.fillRoundRect(340, 270, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 9: {
      display.fillRoundRect(180, 270, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 10: {
      display.fillRoundRect(20, 270, 120, 40, 10, TFT_LIGHTGREY);
      break;
    }
  }

  float* ab = curve.absorbance;
  float* conc = curve.concentration;

  for(int i=0;i<4;i++){
    if(curve.points != 0 && (i+row_offset) < curve.points) {
      display.setCursor(x0, (y + 40*i));
      display.println(i + row_offset + 1); 
      if(ab[i+row_offset] != -1) {
        display.setCursor(x2, (y + 40*i));
        sprintf(a, "%.2f", (double)ab[i + row_offset]);
        display.println(a);
      }
      if(lastPointIsInterpolated == true && (i+row_offset == curve.points - 1))
        display.setTextColor(TFT_RED);
      display.setCursor(x1, (y + 40*i));
      sprintf(c, "%.3f", (double)conc[i + row_offset]);
      display.println(c);
    }
  }

  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(55, 282);
  display.println("BACK");
  display.drawRoundRect(20, 270, 120, 40, 10, TFT_BLACK);
  display.setCursor(218, 282);
  display.println("SAVE");
  display.drawRoundRect(180, 270, 120, 40, 10, TFT_BLACK);
  display.setCursor(358, 282);
  display.println("REGRESS");
  display.drawRoundRect(340, 270, 120, 40, 10, TFT_BLACK);
}

void sctp_lcd_conc_table_clear(uint8_t cursor, uint8_t row_offset, curve_t curve){
  display.fillRect(85, 75, 350, 150, TFT_WHITE);
  switch(cursor) {
    case 0: {
      display.fillRect(195, 75, 75, 35, TFT_WHITE);
      break;
    }
    case 1: {
      display.fillRect(195, 115, 75, 35, TFT_WHITE);
      break;
    }
     case 2: {
      display.fillRect(195, 155, 75, 35, TFT_WHITE);
      break;
    }
    case 3: {
      display.fillRect(195, 195, 75, 35, TFT_WHITE);
      break;
    }
    case 4: {
      display.fillRect(335, 75, 100, 35, TFT_WHITE);
      break;
    }
    case 5: {
      display.fillRect(335, 115, 100, 35, TFT_WHITE);
      break;
    }
     case 6: {
      display.fillRect(335, 155, 100, 35, TFT_WHITE);
      break;
    }
    case 7: {
      display.fillRect(335, 195, 100, 35, TFT_WHITE);
      break;
    }
    case 8: {
      display.fillRoundRect(340, 270, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 9: {
      display.fillRoundRect(180, 270, 120, 40, 10, TFT_WHITE);
      break;
    }
    case 10: {
      display.fillRoundRect(20, 270, 120, 40, 10, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_conc_table_concentration(uint8_t cursor, float concentration){
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1.5);

  switch(cursor) {
    case 0: {
      display.fillRect(195, 75, 75, 35, TFT_LIGHTGREY);
      display.setCursor(200, 80);
      char c[] = "X.XXX";
      sprintf(c, "%.3f", (double)concentration);
      display.println(c);
      break;
    }
    case 1: {
      display.fillRect(195, 115, 75, 35, TFT_LIGHTGREY);
      display.setCursor(200, 120);
      char c[] = "X.XXX";
      sprintf(c, "%.3f", (double)concentration);
      display.println(c);
      break;
    }
    case 2: {
      display.fillRect(195, 155, 75, 35, TFT_LIGHTGREY);
      display.setCursor(200, 160);
      char c[] = "X.XXX";
      sprintf(c, "%.3f", (double)concentration);
      display.println(c);
      break;
    }
    case 3: {
      display.fillRect(195, 195, 75, 35, TFT_LIGHTGREY);
      display.setCursor(200, 200);
      char c[] = "X.XXX";
      sprintf(c, "%.3f", (double)concentration);
      display.println(c);
      break;
    }
  }
}

void sctp_lcd_conc_regress_error(uint8_t cursor)
{
  display.fillRoundRect(180, 160, 120, 40, 10, TFT_LIGHTGREY);
  display.drawRoundRect(180, 160, 120, 40, 10, TFT_BLACK);
  display.setTextColor(TFT_MUSTARD);
  display.setTextSize(1);
  display.setCursor(165, 125);
  display.println("Not enough data!");
  display.setTextColor(TFT_BLACK);
  display.setCursor(228, 175);
  display.println("OK");
}

void sctp_lcd_conc_regress(uint8_t cursor, curve_t curve, bool lastPointIsInterpolated, conc_regression_t * regress_line)
{
  // float x_max = 1.2;
  // float x_min = 0;
  // float y_max = 0.8;
  // float y_min = 0;
  display.setTextColor(TFT_TOSCA);
  display.setTextSize(1);
  display.setCursor(75, 10);
  display.println("CONC VS. ABS");
  display.drawRect(75, 30, 336, 226, TFT_BLACK);

  float* conc = curve.concentration;
  float* ab = curve.absorbance;
  // float conc[] = {0.16, 0.4, 0.86, 1.04}; 
  // float ab[] = {0.08, 0.2, 0.42, 0.52};
  float a_min = 0;
  float a_max = ab[0];
  float c_min = 0;
  float c_max = conc[0];

  for(int i=1;i < curve.points;i++) {
    if (ab[i] >= a_max) a_max = ab[i];
    if (conc[i] >= c_max) c_max = conc[i];
  }
  //c_max = c_max * 1000; //CHANGE LATER MULTIPLICATIONS BY 1000
  ESP_LOGI(TAG, "c_max=%.3f", (double)c_max);

  for(int i=0; i<5; i++) {
    display.setCursor(27, 30 + 53*i);
    display.println(a_max - a_max/4*i);
    char c[] = "X.XXX";
    sprintf(c, "%.3f", (double)(c_min + c_max/4*i));
    display.setCursor(50 + 82*i, 260);
    display.println(c);
  }

  int i = 0;
  int x_px;
  int y_px;

  //float m = 100; //test case
  float m = regress_line->gradient;
  float offset = regress_line->offset;
  for(int i=1;i<329;i++) {
    x_px = 75+i;
    float y = (i * c_max / 330 - offset) / m;
    y_px = 255 - (y - a_min) / (a_max - a_min) * 220;
    display.fillRect(x_px-1, y_px-1, 3, 3, TFT_TOSCA);
  }

  //Scatter plot of points
  while(i < curve.points){
    x_px = (conc[i] - c_min) / (c_max - c_min) * 330 + 75;
    y_px = 255 - (ab[i] - a_min) / (a_max - a_min) * 220;
    if(lastPointIsInterpolated == true && i == curve.points - 1)
      display.fillRect(x_px-4, y_px-4, 9, 9, TFT_RED);
    else
      display.fillRect(x_px-4, y_px-4, 9, 9, TFT_MUSTARD);
    i = i + 1;
  }

  switch(cursor) {
    case 0: {
      break;
    }
    case 1: {
      display.fillRoundRect(180, 280, 120, 30, 10, TFT_LIGHTGREY);
      break;
    }
  }
  display.fillRoundRect(180, 280, 120, 30, 10, TFT_LIGHTGREY);
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(230, 287);
  display.println("OK");
  display.drawRoundRect(180, 280, 120, 30, 10, TFT_BLACK);
}

void sctp_lcd_conc_blank_sampling(uint8_t cursor){
  sctp_lcd_blank_sampling(cursor);
}

void sctp_lcd_conc_blank_waiting(uint8_t cursor, uint16_t result)
{
  sctp_lcd_blank_waiting(cursor, result);
}

void sctp_lcd_conc_blank_clear(uint8_t cursor){
  sctp_lcd_blank_clear(cursor);
}

void sctp_lcd_conc_sample_sampling(uint8_t cursor){
  sctp_lcd_sample_sampling(cursor);
}

void sctp_lcd_conc_sample_waiting(uint8_t cursor, uint16_t result)
{
  sctp_lcd_sample_waiting(cursor, result);
}

void sctp_lcd_conc_sample_clear(uint8_t cursor){
  sctp_lcd_sample_clear(cursor);
}

void sctp_lcd_settings_clear(uint8_t cursor)
{
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  switch(cursor) {
    case 0:
    {
      display.fillRoundRect(120, 60, 245, 40, 10, TFT_WHITE);
      break;
    }
    case 1: {
      display.fillRoundRect(120, 110, 245, 40, 10, TFT_WHITE);
      break;
    }
    case 2: {
      display.fillRoundRect(120, 160, 245, 40, 10, TFT_WHITE);
      break;
    }
  }
}

void sctp_lcd_settings(uint8_t cursor)
{
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1);
  switch(cursor) {
    case 0:
    {
      display.fillRoundRect(120, 60, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 1: {
      display.fillRoundRect(120, 110, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
    case 2: {
      display.fillRoundRect(120, 160, 245, 40, 10, TFT_LIGHTGREY);
      break;
    }
  }
  display.setCursor(145, 75);
  display.println("CHECK CALIBRATION");
  display.drawRoundRect(120, 60, 245, 40, 10, TFT_BLACK);
  display.setCursor(152, 125);
  display.println("LOAD CALIBRATION");
  display.drawRoundRect(120, 110, 245, 40, 10, TFT_BLACK);
  display.setCursor(218, 175);
  display.println("BACK");
  display.drawRoundRect(120, 160, 245, 40, 10, TFT_BLACK);
}

void sctp_lcd_settings_check(calibration_t calibration)
{
  display.fillRect(100, 200, 275, 75, TFT_WHITE);
  display.setTextColor(TFT_MUSTARD);
  display.setTextSize(1);
  char text1[60];
  char text2[60];
  sprintf(text1, "Gain: %.3f, Bias: %.3f,", (double)calibration.gain, (double)calibration.bias);
  sprintf(text2, "Row: %d, Start: %d, Length: %d", calibration.row, calibration.start, calibration.length);
  display.setCursor(128, 225);
  display.println(text1);
  display.setCursor(107, 250);
  display.println(text2);
}

void sctp_lcd_history_list(uint8_t cursor,  uint8_t row_offset, char filenames[60][25]){
  display.setTextColor(TFT_TOSCA);
  display.setTextSize(1.25);
  display.setCursor(60, 20);
  display.println("Filename");
  display.fillRect(40, 50, 420, 210, TFT_WHITE);

  switch(cursor) {
    case 0: {
      display.fillRoundRect(40, 52, 400, 30, 5, TFT_LIGHTGREY);
      break;
    }
    case 1: {
      display.fillRoundRect(40, 87, 400, 30, 5, TFT_LIGHTGREY);
      break;
    }
    case 2: {
      display.fillRoundRect(40, 122, 400, 30, 5, TFT_LIGHTGREY);
      break;
    }
    case 3: {
      display.fillRoundRect(40, 157, 400, 30, 5, TFT_LIGHTGREY);
      break;
    }
    case 4: {
      display.fillRoundRect(40, 192, 400, 30, 5, TFT_LIGHTGREY);
      break;
    }
    case 5: {
      display.fillRoundRect(40, 227, 400, 30, 5, TFT_LIGHTGREY);
      break;
    }
    case 6: {
      display.fillRoundRect(185, 270, 120, 30, 10, TFT_LIGHTGREY);  
      break;
    }
  }
  
  display.setTextSize(1);
  display.setTextColor(TFT_BLACK);

  for(int i=0; i<6; i++) {
   display.setCursor(60, 60 + 35*i);
   display.println(filenames[i+row_offset]);
  }
   
  display.setTextColor(TFT_BLACK);
  display.setCursor(220, 278);
  display.println("BACK");
  display.drawRoundRect(185, 270, 120, 30, 10, TFT_BLACK);  
}

void sctp_lcd_history_list_clear(uint8_t cursor) {
    switch(cursor) {
    case 0: {
      display.fillRoundRect(40, 52, 400, 30, 5, TFT_WHITE);
      break;
    }
    case 1: {
      display.fillRoundRect(40, 87, 400, 30, 5, TFT_WHITE);
      break;
    }
    case 2: {
      display.fillRoundRect(40, 122, 400, 30, 5, TFT_WHITE);
      break;
    }
    case 3: {
      display.fillRoundRect(40, 157, 400, 30, 5, TFT_WHITE);
      break;
    }
    case 4: {
      display.fillRoundRect(40, 192, 400, 30, 5, TFT_WHITE);
      break;
    }
    case 5: {
      display.fillRoundRect(40, 227, 400, 30, 5, TFT_WHITE);
      break;
    }
    case 6: {
      display.fillRoundRect(185, 270, 120, 30, 10, TFT_WHITE);  
      break;
    }
  }
}

void sctp_lcd_battery(uint8_t batt_level) {
  display.fillRoundRect(464, 20, 2, 10, 2, TFT_WHITE);
  display.drawRoundRect(424, 15, 40, 20, 5, TFT_WHITE);
  uint8_t bars = round(batt_level/20);
  switch(bars) {
    case 0: {
      display.fillRoundRect(464, 20, 2, 10, 2, TFT_RED);
      display.drawRoundRect(424, 15, 40, 20, 5, TFT_RED);
      break;
    }
    case 1: {
      display.fillRoundRect(464, 20, 2, 10, 2, TFT_RED);
      display.drawRoundRect(424, 15, 40, 20, 5, TFT_RED);
      display.fillRect(427, 18, 6, 14, TFT_RED);
      break;
    }
    case 2: {
      display.fillRoundRect(464, 20, 2, 10, 2, TFT_MUSTARD);
      display.drawRoundRect(424, 15, 40, 20, 5, TFT_MUSTARD);
      display.fillRect(427, 18, 6, 14, TFT_MUSTARD);
      display.fillRect(434, 18, 6, 14, TFT_MUSTARD);
      break;
    }
    case 3: {
      display.fillRoundRect(464, 20, 2, 10, 2, TFT_MUSTARD);
      display.drawRoundRect(424, 15, 40, 20, 5, TFT_MUSTARD);
      display.fillRect(427, 18, 6, 14, TFT_MUSTARD);
      display.fillRect(434, 18, 6, 14, TFT_MUSTARD);
      display.fillRect(441, 18, 6, 14, TFT_MUSTARD);
      break;
    }
    case 4: {
      display.fillRoundRect(464, 20, 2, 10, 2, TFT_TOSCA);
      display.drawRoundRect(424, 15, 40, 20, 5, TFT_TOSCA);
      display.fillRect(427, 18, 6, 14, TFT_TOSCA);
      display.fillRect(434, 18, 6, 14, TFT_TOSCA);
      display.fillRect(441, 18, 6, 14, TFT_TOSCA);
      display.fillRect(448, 18, 6, 14, TFT_TOSCA);
      break;
    }
    case 5: {
      display.fillRoundRect(464, 20, 2, 10, 2, TFT_TOSCA);
      display.drawRoundRect(424, 15, 40, 20, 5, TFT_TOSCA);
      display.fillRect(427, 18, 6, 14, TFT_TOSCA);
      display.fillRect(434, 18, 6, 14, TFT_TOSCA);
      display.fillRect(441, 18, 6, 14, TFT_TOSCA);
      display.fillRect(448, 18, 6, 14, TFT_TOSCA);
      display.fillRect(455, 18, 6, 14, TFT_TOSCA);
      break;
    }
  }
}