#include "sctp_lcd.h"

#define TFT_TOSCA 0x5D35
#define TFT_MUSTARD 0xD461

static LGFX display;
static LGFX_Sprite sprite(&display);

void sctp_lcd_clear() {
    display.fillScreen(TFT_WHITE);
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

void sctp_lcd_menu_clear(int cursor)
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

void sctp_lcd_menu(int cursor)
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

void sctp_lcd_conc_input_1(int cursor){
  display.setTextColor(TFT_BLACK);
  display.setTextSize(1.25);
  display.setCursor(60, 70);
  display.println("Set wavelength for measurement");
  display.setCursor(60, 120);
  display.println("WAVELENGTH:");
  display.setCursor(240, 120);
  char wl[] = "XXX nm";
  sprintf(wl, "%03d nm", cursor);
  display.println(wl);

  display.setTextColor(TFT_TOSCA);
  display.setTextSize(1);
  display.setCursor(70, 260);
  display.println("Press the arrow keys to set wavelength");
  display.setCursor(150, 285);
  display.println("Press OK to continue");

  display.fillTriangle(400, 135, 410, 120, 420, 135, TFT_BLACK);
  display.fillTriangle(430, 120, 440, 135, 450, 120, TFT_BLACK);
}