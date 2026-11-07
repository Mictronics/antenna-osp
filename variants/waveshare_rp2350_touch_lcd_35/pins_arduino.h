#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf

#define PICO_RP2350B 1  // 48 GPIOs; without it the core rejects GPIO >= 30 (SD CS = 31)

// LEDs (no user LED; GPIO22 is the LCD backlight)
#define PIN_LED        (22u)

// Serial
#define PIN_SERIAL1_TX (44u)
#define PIN_SERIAL1_RX (45u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (21u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)

#define PIN_WIRE1_SDA  (34u)
#define PIN_WIRE1_SCL  (35u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include <generic/common.h>

// Board wiring (Waveshare examples): LCD RST=23 DC=20 BL=22 CS=21; touch RST=24 INT=25;
// touch/IMU/RTC/codec share Wire1; IMU INT1=14; battery ADC=28.
// Wire0 pins are unused placeholders.
