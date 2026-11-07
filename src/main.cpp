#include <Arduino.h>
#include "scpi.h"
#include "ui.h"

// Relay outputs K11..K15 on header pins 6,8,10,12,16 (GPIO0-3,5); GPIO4 is LCD MISO.
static const uint8_t RELAY_PINS[] = {0, 1, 2, 3, 5};
static const size_t NCH = sizeof(RELAY_PINS);
static const uint8_t NROUTES = 6;
// UART1 to the TTL-to-RS485 module (GPIO8 TX / GPIO9 RX are a valid UART1 pair).
#define RS485 Serial2
static const uint8_t RS485_TX = 8, RS485_RX = 9;

// Relay modules energise on a LOW input.
static const bool ACTIVE_LOW = true;
static inline void drive(size_t ch, bool on) { digitalWrite(RELAY_PINS[ch], on != ACTIVE_LOW); }

// RF tree: K1 picks the K2 or K3 branch, K2 = IN1/IN2, K3 = K4 or K5, K4 = IN3/IN4, K5 = IN5/IN6.
// Bit k = relay K(k+1) energised; de-energised (NC) selects the first input of each pair.
static const uint8_t ROUTE_MASK[NROUTES] = {
    0b00000,  // IN1
    0b00010,  // IN2: K2
    0b00001,  // IN3: K1
    0b01001,  // IN4: K1 K4
    0b00101,  // IN5: K1 K3
    0b10101,  // IN6: K1 K3 K5
};

// bit n = K(11+n); bit 5 is the virtual K16 with no hardware behind it.
static uint8_t relays;

uint8_t relays_get() { return relays; }

void relays_apply(uint8_t mask, uint8_t val) {
  relays = (relays & ~mask) | (val & mask);
  for (size_t k = 0; k < NCH; k++) drive(k, relays >> k & 1);
  ui_draw_route();
}

void route_set(uint8_t n) {
  if (n >= 1 && n <= NROUTES) relays_apply(0x1F, ROUTE_MASK[n - 1]);
}

// 0 when the relays match no route (e.g. set remotely).
uint8_t route_get() {
  for (uint8_t i = 0; i < NROUTES; i++)
    if ((relays & 0x1F) == ROUTE_MASK[i]) return i + 1;
  return 0;
}

static void reply(const char *s) { RS485.print(s); }

void setup() {
  relays_apply(0x3F, 0);  // set levels first so pins never glitch a relay on
  for (size_t i = 0; i < NCH; i++) pinMode(RELAY_PINS[i], OUTPUT);
  RS485.setTX(RS485_TX);
  RS485.setRX(RS485_RX);
  RS485.begin(115200);
  ui_init();  // after relays: it draws the active route
}

void loop() {
  static char line[512];
  static size_t len;
  while (RS485.available()) {
    char c = RS485.read();
    if (c == '\n' || c == '\r') {
      if (len) {
        line[len] = 0;
        scpi_handle(line, reply);
        len = 0;
      }
    } else if (len < sizeof line - 1) {
      line[len++] = c;
    }
  }
  static uint32_t last;
  if (millis() - last >= 20) {  // touch poll rate
    last = millis();
    ui_poll();
  }
}
