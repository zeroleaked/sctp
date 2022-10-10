#pragma once

#include <LovyanGFX.h>
#include "sctp_common_types.h"
#include <stdint.h>

#define LCD_PIN_MISO    12
#define LCD_PIN_MOSI    13
#define LCD_PIN_SCLK    14

// #define LCD_PIN_CS      36
// #define LCD_PIN_DC      37
// #define LCD_PIN_BL      47
// #define LCD_PIN_RST     48

#define LCD_PIN_CS      17
#define LCD_PIN_DC      8
#define LCD_PIN_BL      9
#define LCD_PIN_RST     18

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9488     _panel_instance;
    lgfx::Bus_SPI           _bus_instance;   // SPIバスのインスタンス
    lgfx::Light_PWM         _light_instance;

    public:

    // コンストラクタを作成し、ここで各種設定を行います。
    // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
    LGFX(void)
    {
        { // バス制御の設定を行います。
        auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

    // SPIバスの設定
        cfg.spi_host = SPI3_HOST;     // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
        // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
        cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
        cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
        cfg.freq_read  = 8000000;    // 受信時のSPIクロック
        cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
        cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
        // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
        cfg.pin_sclk = LCD_PIN_SCLK;            // SPIのSCLKピン番号を設定
        cfg.pin_mosi = LCD_PIN_MOSI;            // SPIのMOSIピン番号を設定
        cfg.pin_miso = LCD_PIN_MISO;            // SPIのMISOピン番号を設定 (-1 = disable)
        cfg.pin_dc   = LCD_PIN_DC;            // SPIのD/Cピン番号を設定  (-1 = disable)
        // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください

        _bus_instance.config(cfg);    // 設定値をバスに反映します。
        _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
        }

        { // 表示パネル制御の設定を行います。
        auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

        cfg.pin_cs           =    LCD_PIN_CS;  // CSが接続されているピン番号   (-1 = disable)
        cfg.pin_rst          =    LCD_PIN_RST;  // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

        // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

        cfg.panel_width      =   320;  // 実際に表示可能な幅
        cfg.panel_height     =   480;  // 実際に表示可能な高さ
        cfg.offset_x         =     0;  // パネルのX方向オフセット量
        cfg.offset_y         =     0;  // パネルのY方向オフセット量
        cfg.offset_rotation  =     1;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
        cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
        cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
        cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
        cfg.invert           = false;  // パネルの明暗が反転してしまう場合 trueに設定
        cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
        cfg.dlen_16bit       = false;  // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
        cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

    // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
    //    cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
    //    cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ

        _panel_instance.config(cfg);
        }

    //*
        { // バックライト制御の設定を行います。（必要なければ削除）
        auto cfg = _light_instance.config();    // バックライト設定用の構造体を取得します。

        cfg.pin_bl = LCD_PIN_BL;              // バックライトが接続されているピン番号
        cfg.invert = false;           // バックライトの輝度を反転させる場合 true
        cfg.freq   = 44100;           // バックライトのPWM周波数
        cfg.pwm_channel = 7;          // 使用するPWMのチャンネル番号

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
        }

        setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};

void sctp_lcd_clear();

void sctp_lcd_start();

void sctp_lcd_menu(uint8_t cursor);
void sctp_lcd_menu_clear(uint8_t cursor);

void sctp_lcd_spec_blank_sampling(uint8_t cursor);
void sctp_lcd_spec_blank_waiting(uint8_t cursor);
void sctp_lcd_spec_blank_clear(uint8_t cursor);

void sctp_lcd_spec_sample_sampling(uint8_t cursor);
void sctp_lcd_spec_sample_waiting(uint8_t cursor);
void sctp_lcd_spec_sample_clear(uint8_t cursor);

void sctp_lcd_spec_result(uint8_t cursor, float * wavelength, float * absorbance, uint16_t length);
void sctp_lcd_spec_result_clear(uint8_t cursor);
void sctp_lcd_spec_result_cursor(uint8_t cursor);
void sctp_lcd_spec_result_full(float * wavelength, float * absorbance, uint16_t length);

void sctp_lcd_spec_save_saving();
void sctp_lcd_spec_save_finish(char saved_name[20]);
void sctp_lcd_spec_save_finish_cursor(uint8_t cursor);

void sctp_lcd_conc_curves_opening(uint8_t cursor);
void sctp_lcd_conc_curves_list(uint8_t cursor, curve_t curves[6]);
void sctp_lcd_conc_curves_list_cursor(uint8_t cursor);
void sctp_lcd_conc_curves_list_clear(uint8_t cursor);
void sctp_lcd_conc_curves_loading_floats(uint8_t cursor);

void sctp_lcd_conc_wavelength(uint8_t cursor, uint16_t wavelength);
void sctp_lcd_conc_wavelength_clear(uint8_t cursor);
void sctp_lcd_conc_wavelength_number(uint16_t wavelength);

void sctp_lcd_conc_table_opening(uint8_t cursor);
void sctp_lcd_conc_table_cursor(uint8_t cursor, uint8_t row_offset, curve_t curve);
void sctp_lcd_conc_table_clear(uint8_t cursor, uint8_t row_offset, curve_t curve);
void sctp_lcd_conc_table_concentration(uint8_t cursor, float concentration);

void sctp_lcd_conc_blank_waiting(uint8_t cursor);
void sctp_lcd_conc_blank_sampling(uint8_t cursor);
void sctp_lcd_conc_blank_clear(uint8_t cursor);

void sctp_lcd_conc_sample_sampling(uint8_t cursor);
void sctp_lcd_conc_sample_waiting(uint8_t cursor);
void sctp_lcd_conc_sample_clear(uint8_t cursor);

void sctp_lcd_conc_regress(uint8_t cursor, curve_t curve, bool lastPointIsInterpolated, conc_regression_t * regress_line);
void sctp_lcd_conc_regress_error(uint8_t cursor);

void sctp_lcd_history_list(uint8_t cursor,  uint8_t row_offset, char filenames[60][25]);
void sctp_lcd_history_list_clear(uint8_t cursor);

void sctp_lcd_battery(uint8_t batt_level);