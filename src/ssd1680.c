/*
 * MIT License
 *
 * Copyright (c) 2024 permanentnydebet
 */

#include <unistd.h>
#include <string.h>

#include "ssd1680.h"
#include "ssd1680_regmap.h"

typedef struct {
    uint16_t mux : 9;
    uint16_t : 7;
    uint8_t gd : 1;
    uint8_t sm : 1;
    uint8_t tb : 1;
    uint8_t : 5;
} ssd1680_gate_t;

static void _wait_busy(Ssd1680 *disp) {
    while (disp->fn->is_busy() == true)
        disp->fn->delay_ms(2);
}

static void _write(Ssd1680 *disp, ssd1680Register reg, const void *data, size_t data_size) {
    disp->fn->write(reg, (uint8_t *)data, data_size);
}

static void _hw_reset(Ssd1680 *disp) {
    disp->fn->reset(false);
    disp->fn->delay_ms(10);

    disp->fn->reset(true);
    disp->fn->delay_ms(10);

    disp->fn->reset(false);
    disp->fn->delay_ms(10);

    _wait_busy(disp);
    disp->fn->delay_ms(10);
}

static void _sw_reset(Ssd1680 *disp) {
    _write(disp, SSD1680_SW_RESET, NULL, 0);
    _wait_busy(disp);
}

static void set_x_window(Ssd1680 *disp, uint16_t x_start, uint16_t x_stop) {
    uint8_t x_window[2] = {
        x_start & 0x1F,
        x_stop & 0x1F,
    };
    _write(disp, SSD1680_SET_RAM_X_ADDR, x_window, sizeof(x_window));
    _wait_busy(disp);
}

static void set_y_window(Ssd1680 *disp, uint16_t y_start, uint16_t y_stop) {
    uint8_t y_window[4] = {
        y_start & 0xFF,
        (y_start >> 8) & 0x01,
        y_stop & 0xFF,
        (y_stop >> 8) & 0x01,
    };
    _write(disp, SSD1680_SET_RAM_Y_ADDR, y_window, sizeof(y_window));
    _wait_busy(disp);
}

static void set_ram_pos(Ssd1680 *disp, uint16_t x_pos, uint16_t y_pos) {
    uint8_t pos_x_to_set = x_pos & 0x1F;
    uint16_t pos_y_to_set = y_pos & 0x1FF;
    _write(disp, SSD1680_SET_RAM_X_ADDR_CNT, &pos_x_to_set, sizeof(pos_x_to_set));
    _wait_busy(disp);
    _write(disp, SSD1680_SET_RAM_Y_ADDR_CNT, &pos_y_to_set, sizeof(pos_y_to_set));
    _wait_busy(disp);
}

static void setup_gate_driver(Ssd1680 *disp) {
    ssd1680_gate_t gate = {
        .mux = disp->rows_cnt - 1,
        .gd = 0,
        .sm = 0,
        .tb = 0,
    };
    _write(disp, SSD1680_DRIVER_OUTPUT_CTRL, &gate, sizeof(gate));
    _wait_busy(disp);
}

static void setup_border(Ssd1680 *disp) {
    uint8_t border = 0b00000101;
    _write(disp, SSD1680_BORDER_WAVEFORM_CTRL, &border, sizeof(border));
    _wait_busy(disp);
}

static void setup_booster(Ssd1680 *disp) {
    const uint8_t driver_strength = 0b010;
    const uint8_t min_off_time = 0b0100;

    uint8_t booster[4] = {
        (1 << 7) | (driver_strength << 4) | (min_off_time),
        (1 << 7) | (driver_strength << 4) | (min_off_time),
        (1 << 7) | (driver_strength << 4) | (min_off_time),
        0b00000101,
    };
    _write(disp, SSD1680_BOOSTER_SOFT_START_CTRL, booster, sizeof(booster));
    _wait_busy(disp);
}

static void setup_ram(Ssd1680 *disp) {
    // Set data entry mode
    uint8_t data_entry_set;
    uint16_t xwnd_start = 0, xwnd_stop = 0;
    uint16_t ywnd_start = 0, ywnd_stop = 0;

    switch (disp->orientation) {
        case kSsd1680Orientation0Deg:
            data_entry_set = 0b011;
            xwnd_start = 0;
            xwnd_stop = disp->clmn_cnt - 1;
            ywnd_start = 0;
            ywnd_stop = disp->rows_cnt - 1;
            break;
        case kSsd1680Orientation90Deg:
            data_entry_set = 0b110;
            xwnd_start = disp->clmn_cnt - 1;
            xwnd_stop = 0;
            ywnd_start = 0;
            ywnd_stop = disp->rows_cnt - 1;
            break;
        case kSsd1680Orientation180Deg:
            data_entry_set = 0b000;
            xwnd_start = disp->clmn_cnt - 1;
            xwnd_stop = 0;
            ywnd_start = disp->rows_cnt - 1;
            ywnd_stop = 0;
            break;
        case kSsd1680Orientation270Deg:
            data_entry_set = 0b101;
            xwnd_start = 0;
            xwnd_stop = disp->clmn_cnt - 1;
            ywnd_start = disp->rows_cnt - 1;
            ywnd_stop = 0;
            break;
        default:
            break;
    }

    _write(disp, SSD1680_DATA_ENTRY_MODE, &data_entry_set, sizeof(data_entry_set));
    _wait_busy(disp);

    // Set draw window
    set_x_window(disp, xwnd_start, xwnd_stop);
    set_y_window(disp, ywnd_start, ywnd_stop);

    // Setup border waveform
    setup_border(disp);

    // Setup update control
    uint8_t ctrl_1[2] = {0, 0x80};
    _write(disp, SSD1680_DISP_UPDATE_CTRL_1, &ctrl_1, sizeof(ctrl_1));
    _wait_busy(disp);
    uint8_t ctrl_2 = 0xF7;
    _write(disp, SSD1680_DISP_UPDATE_CTRL_2, &ctrl_2, sizeof(ctrl_2));
    _wait_busy(disp);
}

static void init_sequence(Ssd1680 *disp) {
    setup_gate_driver(disp);
    setup_booster(disp);
    setup_ram(disp);
}

static bool check_functions(Ssd1680AccessFunctions *functions) {
    if (functions == NULL || functions->delay_ms == NULL ||
        functions->is_busy == NULL || functions->reset == NULL ||
        functions->write == NULL)
        return false;

    return true;
}

Ssd1680 *ssd1680_init(Ssd1680AccessFunctions *functions, uint16_t res_x, uint16_t res_y, Ssd1680Orientation orientation) {
    if (check_functions(functions) == false)
        return NULL;

    Ssd1680 *disp = malloc(sizeof(Ssd1680));
    if (disp == NULL)
        return NULL;

    disp->fn = functions;
    disp->orientation = orientation;
    disp->res_x = res_x;
    disp->res_y = res_y;

    switch (disp->orientation) {
        case kSsd1680Orientation0Deg:
        case kSsd1680Orientation180Deg:
            disp->clmn_cnt = (res_x + 7) / 8;
            disp->rows_cnt = res_y;
            break;
        case kSsd1680Orientation90Deg:
        case kSsd1680Orientation270Deg:
            disp->clmn_cnt = (res_y + 7) / 8;
            disp->rows_cnt = res_x;
            break;
        default:
            break;
    }

    disp->framebuffer_size = disp->clmn_cnt * disp->rows_cnt;
    _hw_reset(disp);
    _sw_reset(disp);

    init_sequence(disp);
    return disp;
}

void ssd1680_deinit(Ssd1680 *disp) {
    if (disp == NULL)
        return;

    ssd1680_sleep(disp);
    free(disp);
}

void ssd1680_sleep(Ssd1680 *disp) {
    if (disp == NULL)
        return;

    uint8_t mode = 0b01;
    _write(disp, SSD1680_DEEP_SLEEP_MODE, &mode, 1);
}

void ssd1680_wakeup(Ssd1680 *disp) {
    if (disp == NULL)
        return;
    _hw_reset(disp);
}

void ssd1680_send_framebuffer(Ssd1680 *disp, uint8_t *buffer, size_t length, Ssd1680Color color) {
    if (disp == NULL)
        return;

    switch (disp->orientation) {
    case kSsd1680Orientation0Deg:
        set_ram_pos(disp, 0, 0);
        break;
    case kSsd1680Orientation90Deg:
        set_ram_pos(disp, disp->clmn_cnt - 1, 0);
        break;
    case kSsd1680Orientation180Deg:
        set_ram_pos(disp, disp->clmn_cnt - 1, disp->rows_cnt - 1);
        break;
    case kSsd1680Orientation270Deg:
        set_ram_pos(disp, 0, disp->rows_cnt - 1);
        break;
    default:
        break;
    }

    if (color == kSsd1680ColorRed)
        _write(disp, SSD1680_WRITE_RAM_RED, buffer, length);
    else
        _write(disp, SSD1680_WRITE_RAM_BW, buffer, length);

    _wait_busy(disp);
}

void ssd1680_set_refresh_window(Ssd1680 *disp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    if (disp == NULL)
        return;

    switch (disp->orientation) {
        case kSsd1680Orientation0Deg:
            set_x_window(disp, x1 >> 3, x2 >> 3);
            set_y_window(disp, y1, y2);
            break;
        case kSsd1680Orientation90Deg:
            set_x_window(disp, y2 >> 3, y1 >> 3);
            set_y_window(disp, x1, x2);
            break;
        case kSsd1680Orientation180Deg:
            set_x_window(disp, x2 >> 3, x1 >> 3);
            set_y_window(disp, y2, y1);
            break;
        case kSsd1680Orientation270Deg:
            set_x_window(disp, y1 >> 3, y2 >> 3);
            set_y_window(disp, x2, x1);
            break;
        default:
            break;
    }
}

void ssd1680_refresh(Ssd1680 *disp) {
    if (disp == NULL)
        return;

    uint8_t ctrl_2 = 0xF7;
    _write(disp, SSD1680_DISP_UPDATE_CTRL_2, &ctrl_2, sizeof(ctrl_2));
    _write(disp, SSD1680_MASTER_ACTIVATION, NULL, 0);
    _wait_busy(disp);
}

void ssd1680_refresh_window(Ssd1680 *disp) {
    if (disp == NULL)
        return;

    uint8_t ctrl_2 = 0xCC;
    _write(disp, SSD1680_DISP_UPDATE_CTRL_2, &ctrl_2, sizeof(ctrl_2));
    _write(disp, SSD1680_MASTER_ACTIVATION, NULL, 0);
    _wait_busy(disp);
}
