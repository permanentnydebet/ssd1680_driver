/*
 * MIT License
 *
 * Copyright (c) 2024 permanentnydebet
 */

#ifndef SSD1680_SSD1680_REGMAP_H_
#define SSD1680_SSD1680_REGMAP_H_

typedef enum {
    SSD1680_DRIVER_OUTPUT_CTRL            = 0x01,
    SSD1680_GATE_DRIVING_VOLT_CTRL        = 0x03,
    SSD1680_SOURCE_DRIVING_VOLT_CTRL      = 0x04,
    SSD1680_INITIAL_CODE_SETTING_OTP_PROG = 0x08,
    SSD1680_INITIAL_CODE_WRITE_SETTING    = 0x09,
    SSD1680_INITIAL_CODE_READ_SETTING     = 0x0A,
    SSD1680_BOOSTER_SOFT_START_CTRL       = 0x0C,
    SSD1680_DEEP_SLEEP_MODE               = 0x10,
    SSD1680_DATA_ENTRY_MODE               = 0x11,
    SSD1680_SW_RESET                      = 0x12,
    SSD1680_HV_READY_DET                  = 0x14,
    SSD1680_VCI_DET                       = 0x15,
    SSD1680_TEMP_SENS_CTRL                = 0x18,
    SSD1680_TEMP_SENS_CTRL_WRITE          = 0x1A,
    SSD1680_TEMP_SENS_CTRL_READ           = 0x1B,
    SSD1680_TEMP_SENS_CTRL_WRITE_EXT      = 0x1C,
    SSD1680_MASTER_ACTIVATION             = 0x20,
    SSD1680_DISP_UPDATE_CTRL_1            = 0x21,
    SSD1680_DISP_UPDATE_CTRL_2            = 0x22,
    SSD1680_WRITE_RAM_BW                  = 0x24,
    SSD1680_WRITE_RAM_RED                 = 0x26,
    SSD1680_READ_RAM                      = 0x27,
    SSD1680_VCOM_SENSE                    = 0x28,
    SSD1680_VCOM_SENSE_DURATION           = 0x29,
    SSD1680_PROGRAM_VCOM_OTP              = 0x2A,
    SSD1680_VCOM_WRITE_CTRL               = 0x2B,
    SSD1680_VCOM_WRITE                    = 0x2C,
    SSD1680_DISP_OPTS_OTP_READ            = 0x2D,
    SSD1680_USER_ID_READ                  = 0x2E,
    SSD1680_STATUS_BIT_READ               = 0x2F,
    SSD1680_PROGRAM_WS_OTP                = 0x30,
    SSD1680_LOAD_WS_OTP                   = 0x31,
    SSD1680_WRITE_LUT                     = 0x32,
    SSD1680_CRC_CALC                      = 0x34,
    SSD1680_CRC_STATUS_READ               = 0x35,
    SSD1680_PROGRAM_OTP_SELECTION         = 0x36,
    SSD1680_DISP_OPTS_WRITE               = 0x37,
    SSD1680_USER_ID_WRITE                 = 0x38,
    SSD1680_OTP_PROGRAM_MODE              = 0x39,
    SSD1680_BORDER_WAVEFORM_CTRL          = 0x3C,
    SSD1680_END_OPT                       = 0x3F,
    SSD1680_READ_RAM_OPT                  = 0x41,
    SSD1680_SET_RAM_X_ADDR                = 0x44,
    SSD1680_SET_RAM_Y_ADDR                = 0x45,
    SSD1680_AUTO_WRITE_RED_RAM            = 0x46,
    SSD1680_AUTO_WRITE_BW_RAM             = 0x47,
    SSD1680_SET_RAM_X_ADDR_CNT            = 0x4E,
    SSD1680_SET_RAM_Y_ADDR_CNT            = 0x4F,
    SSD1680_NOP                           = 0x7F,
} ssd1680Register;

#endif  // SSD1680_SSD1680_REGMAP_H_
