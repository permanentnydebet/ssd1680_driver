/*
 * MIT License
 *
 * Copyright (c) 2024 permanentnydebet
 */

#ifndef SSD1680_SSD1680_H_
#define SSD1680_SSD1680_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    kSsd1680ColorBlack = 0,
    kSsd1680ColorWhite,
    kSsd1680ColorRed,
    // sentinel
    kSsd1680ColorLast,
} Ssd1680Color;

typedef enum {
    kSsd1680Orientation0Deg,
    kSsd1680Orientation90Deg,
    kSsd1680Orientation180Deg,
    kSsd1680Orientation270Deg,
    // sentinel
    kSsd1680OrientationLast
} Ssd1680Orientation;

typedef struct {
    bool (*write)(const uint8_t command, const uint8_t *data, size_t length);
    void (*delay_ms)(uint32_t delay);
    void (*reset)(bool set);
    bool (*is_busy)(void);
}Ssd1680AccessFunctions;

typedef struct {
    Ssd1680AccessFunctions *fn;
    Ssd1680Orientation orientation;

    uint32_t framebuffer_size;
    uint16_t res_x, res_y;
    uint16_t rows_cnt, clmn_cnt;
    bool refresh_whole;
} Ssd1680;

Ssd1680 *ssd1680_init(Ssd1680AccessFunctions *functions, uint16_t res_x, uint16_t res_y, Ssd1680Orientation orientation);
void ssd1680_deinit(Ssd1680* disp);

void ssd1680_sleep(Ssd1680 *disp);
void ssd1680_wakeup(Ssd1680 *disp);
void ssd1680_send_framebuffer(Ssd1680 *disp, uint8_t *buffer, size_t length, Ssd1680Color color);
void ssd1680_refresh(Ssd1680 *disp);

void ssd1680_set_refresh_window(Ssd1680 *disp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ssd1680_refresh_window(Ssd1680 *disp);

#endif  // SSD1680_SSD1680_H_
