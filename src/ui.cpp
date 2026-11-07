#include "ui.h"
#include "display.h"
#include "touch.h"
#include <SD.h>

// A screen is a background image on SD plus touch widgets. Edit SCREENS to add pages.
enum Kind : uint8_t { ROUTE, NAV };  // ROUTE: select input arg (1..6); NAV: go to screen arg
struct Widget { uint16_t x, y, w, h; Kind kind; uint8_t arg; };
struct Screen { const char *bg; const Widget *w; size_t n; };

// Landscape 480x320. Route buttons: 3x2 grid, 146x115 px; nav button along the bottom.
#define RT(i) {(uint16_t)(10 + 157 * (((i) - 1) % 3)), (uint16_t)(10 + 125 * (((i) - 1) / 3)), 146, 115, ROUTE, i}
static const Widget MAIN_W[] = {RT(1), RT(2), RT(3), RT(4), RT(5), RT(6), {10, 260, 460, 50, NAV, 1}};
static const Widget SETTINGS_W[] = {{10, 260, 460, 50, NAV, 0}};

static const Screen SCREENS[] = {
    {"/main.rgb565", MAIN_W, sizeof(MAIN_W) / sizeof(Widget)},
    {"/settings.rgb565", SETTINGS_W, sizeof(SETTINGS_W) / sizeof(Widget)},
};

static const uint16_t FRAME = 4;  // active-route frame thickness, inside the button
static const uint16_t COL_ON = 0x07E0, COL_BG = 0x2104, COL_BTN = 0x4208;

static uint8_t cur;
static uint8_t shown;  // route currently marked on screen, 0 = none
static bool pressed;
static bool ready;  // display initialised; relays may change before that
static File bg;

// Repaint a rect from the background image (or a plain button colour without one).
static void restoreRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  static uint8_t buf[1280];  // largest piece: the 146x4 frame edge
  if (!bg) return display_fill(x, y, w, h, COL_BTN);
  for (uint16_t r = 0; r < h; r++) {
    bg.seek(((uint32_t)(y + r) * LCD_W + x) * 2);
    bg.read(buf + r * w * 2, w * 2);
  }
  display_blit(x, y, w, h, buf);
}

// Draw (fill = true) or erase the four frame edges of a route button.
static void frame(const Widget &b, bool fill) {
  const uint16_t e[4][4] = {{b.x, b.y, b.w, FRAME},
                            {b.x, (uint16_t)(b.y + b.h - FRAME), b.w, FRAME},
                            {b.x, (uint16_t)(b.y + FRAME), FRAME, (uint16_t)(b.h - 2 * FRAME)},
                            {(uint16_t)(b.x + b.w - FRAME), (uint16_t)(b.y + FRAME), FRAME,
                             (uint16_t)(b.h - 2 * FRAME)}};
  for (auto &r : e) fill ? display_fill(r[0], r[1], r[2], r[3], COL_ON) : restoreRect(r[0], r[1], r[2], r[3]);
}

static void mark(uint8_t route, bool on) {
  const Screen &s = SCREENS[cur];
  for (size_t i = 0; i < s.n; i++)
    if (s.w[i].kind == ROUTE && s.w[i].arg == route) frame(s.w[i], on);
}

void ui_draw_route() {
  if (!ready) return;
  uint8_t r = route_get();
  if (r == shown) return;
  mark(shown, false);
  mark(r, true);
  shown = r;
}

static void drawScreen() {
  const Screen &s = SCREENS[cur];
  static uint8_t strip[LCD_W * 2 * 16];  // 16 rows
  if (bg) bg.close();
  bg = SD.open(s.bg, "r");
  if (bg) {
    bg.seek(0);
    for (uint16_t y = 0; y < LCD_H; y += 16) {
      if (bg.read(strip, sizeof(strip)) != (int)sizeof(strip)) break;
      display_blit(0, y, LCD_W, 16, strip);
    }
  } else {  // no image: plain buttons so the UI is still usable
    display_fill(0, 0, LCD_W, LCD_H, COL_BG);
    for (size_t i = 0; i < s.n; i++) display_fill(s.w[i].x, s.w[i].y, s.w[i].w, s.w[i].h, COL_BTN);
  }
  shown = 0;
  ui_draw_route();
}

void ui_init() {
  display_init();
  touch_init();
  SPI1.setSCK(26);
  SPI1.setTX(27);
  SPI1.setRX(28);
  SD.begin(31, 25000000, SPI1);  // 25 MHz; lower if reads fail. Missing card just means plain buttons
  ready = true;
  drawScreen();
}

void ui_poll() {
  uint16_t x, y;
  bool down = touch_read(x, y);
  if (down && !pressed) {  // act on the press edge only
    const Screen &s = SCREENS[cur];
    for (size_t i = 0; i < s.n; i++) {
      const Widget &w = s.w[i];
      if (x < w.x || x >= w.x + w.w || y < w.y || y >= w.y + w.h) continue;
      if (w.kind == ROUTE) {
        route_set(w.arg);
      } else {
        cur = w.arg;
        drawScreen();
      }
      break;
    }
  }
  pressed = down;
}
