#include "ui.h"
#include "display.h"
#include "touch.h"
#include <SD.h>

// A screen is a background image on SD plus touch widgets. Edit SCREENS to add pages.
enum Kind : uint8_t { ROUTE, NAV, BRIGHT, LOCAL };  // ROUTE: input arg (1..6); NAV: screen arg; BRIGHT: backlight level arg (1..4); LOCAL: leave remote lock
struct Widget { uint16_t x, y, w, h; Kind kind; uint8_t arg; };
struct Screen { const char *bg;  // SD base name: /name.rgb565, or /name-hc.rgb565 / /name-light.rgb565
   const Widget *w; size_t n; };

// Landscape 480x320. Route buttons: 3x2 grid, 146x115 px; nav button along the bottom.
#define RT(i) {(uint16_t)(10 + 157 * (((i) - 1) % 3)), (uint16_t)(10 + 125 * (((i) - 1) / 3)), 146, 115, ROUTE, i}
static const Widget MAIN_W[] = {RT(1), RT(2), RT(3), RT(4), RT(5), RT(6), {10, 260, 460, 50, NAV, 1}};
#define BR(i, x) {x, 100, 108, 60, BRIGHT, i}  // x from the design (uneven 117/118 px pitch)
static const Widget SETTINGS_W[] = {BR(1, 10), BR(2, 127), BR(3, 245), BR(4, 362), {10, 260, 460, 50, NAV, 0}};
static const uint8_t BRIGHT_PCT[] = {25, 50, 75, 100};  // index = level - 1
static const char *SETTINGS_FILE = "/bright.txt";

static const Widget REMOTE_W[] = {{0, 0, 480, 320, LOCAL, 0}};  // any touch returns to local
static const uint8_t REMOTE_SCREEN = 2;

static const Screen SCREENS[] = {
    {"/main", MAIN_W, sizeof(MAIN_W) / sizeof(Widget)},
    {"/settings", SETTINGS_W, sizeof(SETTINGS_W) / sizeof(Widget)},
    {"/remote", REMOTE_W, 1},
};

static const uint16_t FRAME = 4;  // active-route frame thickness, inside the button
static const uint16_t COL_ON = 0x07E0, COL_BG = 0x2104, COL_BTN = 0x4208;

static uint8_t cur;
static uint8_t prev;  // screen to return to after remote lock
static uint8_t shown;  // route currently marked on screen, 0 = none
static uint8_t theme;  // index into THEME_CH/THEME_SUFFIX, set via SCPI DISP:THEM
static const char THEME_CH[] = "NHL";  // settings-file char: normal, high contrast, light
static const char *const THEME_SUFFIX[] = {"", "-hc", "-light"};
static uint8_t level = 4;  // backlight level 1..4
static bool pressed;
static bool asleep;  // backlight off after idle
static uint32_t lastTouch;
static const uint32_t SLEEP_MS = 300000;  // idle time before backlight off; only touch counts, not SCPI
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

static void mark(Kind k, uint8_t arg, bool on) {
  const Screen &s = SCREENS[cur];
  for (size_t i = 0; i < s.n; i++)
    if (s.w[i].kind == k && s.w[i].arg == arg) frame(s.w[i], on);
}

static void drawScreen();

static const uint8_t SETTINGS_SCREEN = 1;
static const Widget BATT_BOX = {408, 55, 62, 40, NAV, 0};  // kind/arg unused
static uint8_t batt = 4;  // icon index 0..4 = 0/25/50/75/100 %

// Settings screen only: /batt-NN[-theme].rgb565 is a 62x40 image with the theme background baked in.
static void drawBattery() {
  if (cur != SETTINGS_SCREEN) return;
  char path[32];
  snprintf(path, sizeof path, "/batt-%u%s.rgb565", batt * 25, THEME_SUFFIX[theme]);
  File f = SD.open(path, "r");
  if (!f) return;
  static uint8_t buf[62 * 40 * 2];
  bool ok = f.read(buf, sizeof buf) == (int)sizeof buf;
  f.close();
  if (ok) display_blit(BATT_BOX.x, BATT_BOX.y, BATT_BOX.w, BATT_BOX.h, buf);
}

void ui_battery(uint8_t pct) {
  uint8_t i = (min<uint8_t>(pct, 100) + 12) / 25;  // nearest step
  if (i == batt) return;
  batt = i;
  if (ready) drawBattery();
}

static void brightness_load() {
  File f = SD.open(SETTINGS_FILE, "r");
  if (f) {
    int v = f.read();
    if (v >= '1' && v <= '4') level = v - '0';
    const char *t = strchr(THEME_CH, f.read());
    theme = t && *t ? t - THEME_CH : 0;
    f.close();
  }
  display_backlight(BRIGHT_PCT[level - 1]);
}

static void settings_save() {
  SD.remove(SETTINGS_FILE);  // FILE_WRITE appends
  File f = SD.open(SETTINGS_FILE, FILE_WRITE);
  if (f) {
    f.write('0' + level);
    f.write((uint8_t)THEME_CH[theme]);
    f.close();
  }
}

static void brightness_set(uint8_t l) {
  mark(BRIGHT, level, false);
  level = l;
  mark(BRIGHT, level, true);
  display_backlight(BRIGHT_PCT[level - 1]);
  settings_save();
}

static const char *PATHS_FILE = "/paths.txt";

void paths_write(const char *text) {
  SD.remove(PATHS_FILE);
  File f = SD.open(PATHS_FILE, FILE_WRITE);
  if (f) {
    f.print(text);
    f.close();
  }
}

size_t paths_read(char *buf, size_t cap) {
  size_t n = 0;
  File f = SD.open(PATHS_FILE, "r");
  if (f) {
    n = f.read((uint8_t *)buf, cap - 1);
    f.close();
  }
  buf[n] = 0;
  return n;
}

uint8_t theme_get() { return theme; }

void theme_set(uint8_t t) {
  if (t == theme || t > 2) return;
  theme = t;
  settings_save();
  if (ready) drawScreen();
}

void ui_draw_route() {
  if (!ready) return;
  uint8_t r = route_get();
  if (r == shown) return;
  mark(ROUTE, shown, false);
  mark(ROUTE, r, true);
  shown = r;
}

static bool blit(File &f) {
  static uint8_t strip[LCD_W * 2 * 16];  // 16 rows
  if (!f) return false;
  f.seek(0);
  for (uint16_t y = 0; y < LCD_H; y += 16) {
    if (f.read(strip, sizeof(strip)) != (int)sizeof(strip)) break;
    display_blit(0, y, LCD_W, 16, strip);
  }
  return true;
}

static void drawScreen() {
  const Screen &s = SCREENS[cur];
  if (bg) bg.close();
  char path[32];
  snprintf(path, sizeof path, "%s%s.rgb565", s.bg, THEME_SUFFIX[theme]);
  bg = SD.open(path, "r");
  if (blit(bg)) {
  } else {  // no image: plain buttons so the UI is still usable
    display_fill(0, 0, LCD_W, LCD_H, COL_BG);
    for (size_t i = 0; i < s.n; i++) display_fill(s.w[i].x, s.w[i].y, s.w[i].w, s.w[i].h, COL_BTN);
  }
  shown = 0;
  mark(BRIGHT, level, true);
  drawBattery();
  ui_draw_route();
}

void ui_remote(bool on) {
  if (!ready || on == (cur == REMOTE_SCREEN)) return;
  if (on) prev = cur;
  cur = on ? REMOTE_SCREEN : prev;
  drawScreen();
}

void ui_init() {
  display_init();
  touch_init();
  SPI1.setSCK(26);
  SPI1.setTX(27);
  SPI1.setRX(28);
  SD.begin(31, 25000000, SPI1);  // 25 MHz; lower if reads fail. Missing card just means plain buttons
  brightness_load();
  char path[32];
  snprintf(path, sizeof path, "/splash%s.rgb565", (const char *const[]){"-dark", "-hc", "-light"}[theme]);
  File sp = SD.open(path, "r");
  if (blit(sp)) delay(3000);
  sp.close();
  ready = true;
  lastTouch = millis();
  drawScreen();
}

void ui_poll() {
  uint16_t x, y;
  bool down = touch_read(x, y);
  if (down) lastTouch = millis();
  if (asleep) {
    if (!down) return;
    asleep = false;  // waking touch is swallowed, not passed to widgets
    display_backlight(BRIGHT_PCT[level - 1]);
    pressed = true;
    return;
  }
  if (!down && millis() - lastTouch > SLEEP_MS) {
    asleep = true;
    display_backlight(0);
    return;
  }
  if (down && !pressed) {  // act on the press edge only
    const Screen &s = SCREENS[cur];
    for (size_t i = 0; i < s.n; i++) {
      const Widget &w = s.w[i];
      if (x < w.x || x >= w.x + w.w || y < w.y || y >= w.y + w.h) continue;
      if (w.kind == ROUTE) {
        route_set(w.arg);
      } else if (w.kind == LOCAL) {
        ui_remote(false);
      } else if (w.kind == BRIGHT) {
        brightness_set(w.arg);
      } else {
        cur = w.arg;
        drawScreen();
      }
      break;
    }
  }
  pressed = down;
}

uint32_t ui_last_touch() { return lastTouch; }
