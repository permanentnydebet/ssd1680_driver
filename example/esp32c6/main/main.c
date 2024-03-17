/*
 * MIT License
 *
 * Copyright (c) 2024 permanentnydebet
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "ssd1680.h"
#include "epd.h"

#include "TimesNewRoman32px.h"
#include "FontLiberationSerif12.h"

#define EPD_SPI    SPI2_HOST

#define PIN_NUM_MISO    2
#define PIN_NUM_MOSI    7
#define PIN_NUM_CLK     6
#define PIN_NUM_CS      18

#define PIN_NUM_DC      10
#define PIN_NUM_BUSY    11
#define PIN_NUM_RESET   0

spi_device_handle_t spi = { 0 };

bool SpiInit(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = 2,
        .mosi_io_num = 7,
        .sclk_io_num = 6,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 256
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 100 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
        .pre_cb = NULL,
    };

    //Initialize the SPI bus
    if (spi_bus_initialize(EPD_SPI, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK)
        return false;

    if (spi_bus_add_device(EPD_SPI, &devcfg, &spi) != ESP_OK)
        return false;

    return true;
}

static bool SpiWrite(const uint8_t command, const uint8_t *buffer, size_t length) {
    spi_transaction_t packet = {0};

    packet.length = 8;
    packet.tx_buffer = &command;

    gpio_set_level(PIN_NUM_DC, 0);
    esp_err_t status = spi_device_polling_transmit(spi, &packet);
    gpio_set_level(PIN_NUM_DC, 1);

    if (status != ESP_OK) {
        printf("Fail on SPI write: %s\r\n", esp_err_to_name(status));
        return false;
    }

    if (buffer == NULL)
        return true;

    packet.length = 8 * length;
    packet.tx_buffer = buffer;

    status = spi_device_polling_transmit(spi, &packet);
    if (status != ESP_OK) {
        printf("Fail on SPI write: %s\r\n", esp_err_to_name(status));
        return false;
    }

    return true;
}

bool GpioInit(void) {
    gpio_config_t gpio_dc_conf = {
        .pin_bit_mask    = (uint64_t)1 << PIN_NUM_DC,
        .mode            = GPIO_MODE_OUTPUT,
        .intr_type       = GPIO_INTR_DISABLE,
        .pull_down_en    = 0,
        .pull_up_en      = 0
    };

    if (gpio_config(&gpio_dc_conf) != ESP_OK)
        return false;

    gpio_config_t gpio_busy_conf = {
        .pin_bit_mask    = (uint64_t)1 << PIN_NUM_BUSY,
        .mode            = GPIO_MODE_INPUT,
        .intr_type       = GPIO_INTR_DISABLE,
        .pull_down_en    = 1,
        .pull_up_en      = 0
    };

    if (gpio_config(&gpio_busy_conf) != ESP_OK)
        return false;

    gpio_config_t gpio_reset_conf = {
        .pin_bit_mask    = (uint64_t)1 << PIN_NUM_RESET,
        .mode            = GPIO_MODE_OUTPUT,
        .intr_type       = GPIO_INTR_DISABLE,
        .pull_down_en    = 0,
        .pull_up_en      = 0
    };

    if (gpio_config(&gpio_reset_conf) != ESP_OK)
        return false;

    return true;
}

void DelayMs(uint32_t delay) {
    vTaskDelay(pdMS_TO_TICKS(delay));
}

bool IsBusy(void) {
  vTaskDelay(1);
  return gpio_get_level(PIN_NUM_BUSY) == 1;
}

void Reset(bool state) {
    gpio_set_level(PIN_NUM_RESET, !state);
}

void app_main(void) {
    if (SpiInit() == false)
        printf("spi init error!\n\r");

    if (GpioInit() == false)
        printf("gpio init error!\n\r");

    Ssd1680AccessFunctions func = {
      .write = SpiWrite,
      .delay_ms = DelayMs,
      .is_busy = IsBusy,
      .reset = Reset
    };

  Ssd1680 *display = ssd1680_init(&func, 250, 122, kSsd1680Orientation90Deg);  // ujednolicic zmiane kierunku

  if (display == NULL) {
    printf("Can't alloc memory for dispaly!\n\r");
    goto error;
  }

  EpdImage *image = EPD_NewImage(250, 122, kEpdOrientation90Deg, kEpdColorRed); // tak samo tutaj

  if (image == NULL) {
    printf("Can't alloc memory for image!\n\r");
    goto error;
  }

  EPD_Fill(image, kEpdColorWhite);

  ssd1680_send_framebuffer(display, image->image_bw, image->image_size, kSsd1680ColorBlack);
  ssd1680_send_framebuffer(display, image->image_red, image->image_size, kSsd1680ColorRed);

  EPD_ShowString(image, 15, 15, "Test", &TimesNewRoman32px, kEpdColorRed);
  EPD_ShowString(image, 15, 47, "~!@#$%%^&*()_?><:{[}]}", &LiberationSerif20px, kEpdColorBlack);

  ssd1680_send_framebuffer(display, image->image_red, image->image_size, kSsd1680ColorRed);
  ssd1680_send_framebuffer(display, image->image_bw, image->image_size, kSsd1680ColorBlack);
  ssd1680_refresh(display);

  printf("Done.");

error:
  while (1) {
    DelayMs(1000);
  }
}
