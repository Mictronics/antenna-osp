// R&S OSP-style SCPI subset emulating frame F01 with one OSP-B101 (6 SPDT) in slot M01.
// K11..K15 are real relays; K16 is a stored-only virtual relay (the hardware has 5).
#include "scpi.h"
#include <Arduino.h>
#include <Vrekrer_scpi_parser.h>  // header-only: include from this file only
#include <ctype.h>
#include <initializer_list>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifndef SCPI_IDN
#define SCPI_IDN "DIY,OSP-EMU,000001,0.3"  // change if ELEKTRA rejects this identity
#endif

static const int MAXPATH = 24, MAXITEM = 16;
typedef void (*Out)(const char *);

// ---- error queue and status registers ----
static struct { int code; const char *msg; } errq[8];
static int errn;
static uint8_t esr, ese, sre;

static void err(int code, const char *msg) {
  esr |= code > -200 ? 0x20 : code > -300 ? 0x10 : 0x08;
  if (errn < 8) { errq[errn].code = code; errq[errn].msg = msg; errn++; }
}
static void errPop(char *buf, size_t n) {
  if (!errn) { snprintf(buf, n, "0,\"No error\""); return; }
  snprintf(buf, n, "%d,\"%s\"", errq[0].code, errq[0].msg);
  memmove(errq, errq + 1, sizeof(errq[0]) * --errn);
}

static void say(Stream &s, const char *fmt, ...) {
  static char buf[1300];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf, sizeof buf - 1, fmt, ap);
  va_end(ap);
  if (n > (int)sizeof buf - 2) n = sizeof buf - 2;
  buf[n] = '\n';
  buf[n + 1] = 0;
  s.print(buf);
}

// ---- paths (RAM only) ----
struct Path { char name[40]; uint8_t mask, val; bool used; };
static Path paths[MAXPATH];
static char lastPath[40] = "<Reset State>";
static bool bulk;

static Path *findPath(const char *name) {
  for (auto &p : paths) if (p.used && !strcmp(p.name, name)) return &p;
  return nullptr;
}

// ---- helpers ----
// tok equals the pattern's short form (leading uppercase run) or its long form, any case.
static bool kw(const char *tok, size_t len, const char *pat) {
  size_t sh = 0, pl = strlen(pat);
  while (sh < pl && !islower((unsigned char)pat[sh])) sh++;
  for (size_t n : {sh, pl}) {
    if (len != n) continue;
    size_t i = 0;
    while (i < n && toupper((unsigned char)tok[i]) == toupper((unsigned char)pat[i])) i++;
    if (i == n) return true;
  }
  return false;
}

static bool isQuery(SCPI_C &c) {
  const char *l = c.Last();
  return l && l[0] && l[strlen(l) - 1] == '?';
}

// Parameter i; null if absent.
static char *par(SCPI_P &p, uint8_t i) { return i < p.Size() ? p[i] : nullptr; }

// ---- channel lists ----
struct Item { uint8_t idx, state; };

// The manual is inconsistent for the B101 (01..06 vs K11..K16 as 11..16); accept both.
static int relIdx(int ee) { return ee >= 1 && ee <= 6 ? ee - 1 : ee >= 11 && ee <= 16 ? ee - 11 : -1; }

// Parses "(@F01M01(0112,0204:0206),...)". Returns item count, or -1 with the error queued.
static int parseList(const char **pp, Item *it) {
  const char *p = *pp;
  while (*p == ' ') p++;
  if (p[0] != '(' || p[1] != '@') { err(-102, "Syntax error"); return -1; }
  p += 2;
  int n = 0;
  while (*p && *p != ')') {
    if (*p == ',' || *p == ' ') { p++; continue; }
    int f = 0, m = 0, used = 0;
    if (sscanf(p, "%*1[Ff]%2d%*1[Mm]%2d(%n", &f, &m, &used) != 2 || !used) {
      err(-102, "Syntax error");
      return -1;
    }
    if (f != 1 || m != 1) { err(-200, "Execution error;module not present"); return -1; }
    p += used;
    while (*p && *p != ')') {
      if (*p == ',' || *p == ' ') { p++; continue; }
      char a[8] = {0}, b[8] = {0};
      int la = 0, lb = 0;
      while (isdigit((unsigned char)*p) && la < 7) a[la++] = *p++;
      if (*p == ':') { p++; while (isdigit((unsigned char)*p) && lb < 7) b[lb++] = *p++; }
      if ((la != 2 && la != 4 && la != 5) || (*p && *p != ',' && *p != ')') || (lb && lb != 4 && lb != 5)) {
        err(-102, "Syntax error");
        return -1;
      }
      int ee = atoi(a + la - 2), e2 = lb ? atoi(b + lb - 2) : ee;
      char s[4] = {0};
      if (la > 2) memcpy(s, a, la - 2);
      int state = atoi(s);
      for (int e = ee; e <= e2; e++) {
        int idx = relIdx(e);
        if (idx < 0 || state > 1 || n >= MAXITEM) { err(-222, "Data out of range"); return -1; }
        it[n++] = {(uint8_t)idx, (uint8_t)state};
      }
    }
    if (*p != ')') { err(-102, "Syntax error"); return -1; }
    p++;
  }
  if (*p != ')') { err(-102, "Syntax error"); return -1; }
  *pp = p + 1;
  return n;
}

// Reads a quoted string at *pp ("..." or '...') and advances past it.
static bool parseStr(const char **pp, char *dst, size_t cap) {
  const char *p = *pp;
  while (*p == ' ') p++;
  char q = *p;
  if (q != '"' && q != '\'') { err(-102, "Syntax error"); return false; }
  size_t n = 0;
  for (p++; *p && *p != q; p++) if (n + 1 < cap) dst[n++] = *p;
  if (*p != q) { err(-102, "Syntax error"); return false; }
  dst[n] = 0;
  *pp = p + 1;
  return true;
}

// A channel list or a quoted path name, as relay items. Returns count or -1.
static int resolve(const char **pp, Item *it, char *pathName) {
  const char *p = *pp;
  while (*p == ' ') p++;
  if (*p == '"' || *p == '\'') {
    char name[40];
    if (!parseStr(&p, name, sizeof name)) return -1;
    Path *pt = findPath(name);
    if (!pt) { err(-200, "Execution error;path not found"); return -1; }
    int n = 0;
    for (int i = 0; i < 6; i++)
      if (pt->mask >> i & 1) it[n++] = {(uint8_t)i, (uint8_t)(pt->val >> i & 1)};
    if (pathName) strcpy(pathName, name);
    *pp = p;
    return n;
  }
  if (pathName) strcpy(pathName, "<Individual Settings>");
  return parseList(pp, it);
}

static void toMaskVal(const Item *it, int n, uint8_t *mask, uint8_t *val) {
  *mask = *val = 0;
  for (int i = 0; i < n; i++) {
    *mask |= 1 << it[i].idx;
    *val = (*val & ~(1 << it[i].idx)) | (it[i].state << it[i].idx);
  }
}

static void fmtList(char *buf, size_t cap, uint8_t mask, uint8_t val) {
  size_t n = snprintf(buf, cap, "(@F01M01(");
  bool first = true;
  for (int i = 0; i < 6 && n < cap; i++)
    if (mask >> i & 1) {
      n += snprintf(buf + n, cap - n, "%s%02d%02d", first ? "" : ",", val >> i & 1, 11 + i);
      first = false;
    }
  if (n < cap) snprintf(buf + n, cap - n, "))");
}

static void doReset() {
  relays_apply(0x3F, 0);
  strcpy(lastPath, "<Reset State>");
}

// ---- IEEE 488.2 common commands ----
static void cIdn(SCPI_C, SCPI_P, Stream &s) { say(s, "%s", SCPI_IDN); }
static void cRst(SCPI_C, SCPI_P, Stream &) { doReset(); }
static void cCls(SCPI_C, SCPI_P, Stream &) { errn = 0; esr = 0; }
static void cOpc(SCPI_C c, SCPI_P, Stream &s) { if (isQuery(c)) say(s, "1"); else esr |= 1; }
static void cNop(SCPI_C, SCPI_P, Stream &) {}
static void cZero(SCPI_C, SCPI_P, Stream &s) { say(s, "0"); }  // *PSC? *TST? *OPT?
static void cEse(SCPI_C c, SCPI_P p, Stream &s) { if (isQuery(c)) say(s, "%d", ese); else if (par(p, 0)) ese = atoi(par(p, 0)); }
static void cSre(SCPI_C c, SCPI_P p, Stream &s) { if (isQuery(c)) say(s, "%d", sre); else if (par(p, 0)) sre = atoi(par(p, 0)) & ~0x40; }
static void cEsr(SCPI_C, SCPI_P, Stream &s) { say(s, "%d", esr); esr = 0; }
static void cStb(SCPI_C, SCPI_P, Stream &s) { say(s, "%d", (errn ? 0x04 : 0) | ((esr & ese) ? 0x20 : 0)); }

// ---- SYSTem / DIAGnostic ----
static void cErrNext(SCPI_C, SCPI_P, Stream &s) {
  char b[100];
  errPop(b, sizeof b);
  say(s, "%s", b);
}
static void cErrAll(SCPI_C, SCPI_P, Stream &s) {
  char o[600], one[100];
  size_t n = 0;
  do {
    errPop(one, sizeof one);
    n += snprintf(o + n, sizeof o - n, "%s%s", n ? "," : "", one);
  } while (errn && n < sizeof o - 110);
  say(s, "%s", o);
}
static void cPreset(SCPI_C, SCPI_P, Stream &) { doReset(); }
static void cVersion(SCPI_C, SCPI_P, Stream &s) { say(s, "1999.0"); }
static void cHwInfo(SCPI_C, SCPI_P, Stream &s) {
  say(s, "\"F01|OSP230|000001|0000.0000.00|0|01.00\",\"F01M00|OSPMAINBOARD|000001|0000.0000.00|0|01.00\","
         "\"F01M01|OSP-B101|000001|0000.0000.00|0|01.00\"");
}

// ---- ROUTe ----
static void cClose(SCPI_C c, SCPI_P p, Stream &s) {
  Item it[MAXITEM];
  const char *a = par(p, 0);
  if (!a) return err(-109, "Missing parameter");
  char pn[40];
  int n = resolve(&a, it, isQuery(c) ? nullptr : pn);
  if (n < 0) return;
  if (!isQuery(c)) {
    uint8_t m, v;
    toMaskVal(it, n, &m, &v);
    relays_apply(m, v);
    snprintf(lastPath, sizeof lastPath, "%s", pn);
    return;
  }
  const char *w = par(p, 1);
  size_t wl = w ? strlen(w) : 0;
  bool val = w && kw(w, wl, "VALue"), summ = w && kw(w, wl, "SUMMary");
  uint8_t cur = relays_get();
  char o[80];
  size_t k = 0;
  bool all = true;
  for (int i = 0; i < n; i++) {
    int pos = cur >> it[i].idx & 1;
    bool match = pos == it[i].state;
    all = all && match;
    k += snprintf(o + k, sizeof o - k, "%s%d", i ? "," : "", val ? pos : match);
  }
  say(s, "%s", summ ? (all ? "1" : "0") : o);
}

static void cCombined(SCPI_C, SCPI_P p, Stream &) {
  uint8_t m = 0, v = 0;
  for (uint8_t i = 0; i < p.Size(); i++) {
    char name[40];
    const char *a = par(p, i);
    if (!parseStr(&a, name, sizeof name)) return;
    Path *pt = findPath(name);
    if (!pt) return err(-200, "Execution error;path not found");
    m |= pt->mask;
    v = (v & ~pt->mask) | pt->val;
  }
  relays_apply(m, v);
  strcpy(lastPath, "<Individual Settings>");
}

static void cPathDef(SCPI_C c, SCPI_P p, Stream &s) {
  char name[40];
  const char *a = par(p, 0);
  if (!a) return err(-109, "Missing parameter");
  if (!parseStr(&a, name, sizeof name)) return;
  Path *pt = findPath(name);
  if (isQuery(c)) {
    if (!pt) return err(-200, "Execution error;path not found");
    char o[100];
    fmtList(o, sizeof o, pt->mask, pt->val);
    say(s, "%s", o);
    return;
  }
  const char *l = par(p, 1);
  if (!l) return err(-109, "Missing parameter");
  Item it[MAXITEM];
  int n = parseList(&l, it);
  if (n < 0) return;
  for (int i = 0; !pt && i < MAXPATH; i++) if (!paths[i].used) pt = &paths[i];
  if (!pt) return err(-321, "Out of memory");
  pt->used = true;
  snprintf(pt->name, sizeof pt->name, "%s", name);
  toMaskVal(it, n, &pt->mask, &pt->val);
}

static void cPathDel(SCPI_C, SCPI_P p, Stream &) {
  for (uint8_t i = 0; i < p.Size(); i++) {
    char name[40];
    const char *a = par(p, i);
    if (!parseStr(&a, name, sizeof name)) return;
    if (Path *pt = findPath(name)) pt->used = false;
  }
}
static void cPathDelAll(SCPI_C, SCPI_P, Stream &) { for (auto &pt : paths) pt.used = false; }

static void cPathCat(SCPI_C, SCPI_P p, Stream &s) {
  const char *a = par(p, 0);
  bool active = a && kw(a, strlen(a), "ACTive");
  uint8_t cur = relays_get();
  char o[MAXPATH * 44];
  size_t n = 0;
  for (auto &pt : paths)
    if (pt.used && (!active || (cur & pt.mask) == pt.val))
      n += snprintf(o + n, sizeof o - n, "%s\"%s\"", n ? "," : "", pt.name);
  say(s, "%s", n ? o : "\"\"");
}

static void cPathLast(SCPI_C, SCPI_P, Stream &s) { say(s, lastPath[0] == '<' ? "%s" : "\"%s\"", lastPath); }

static void cPathMult(SCPI_C c, SCPI_P p, Stream &s) {
  const char *a = par(p, 0);
  if (isQuery(c)) say(s, "%d", bulk);
  else if (a) bulk = !strcasecmp(a, "ON") || !strcmp(a, "1");
}

static void cUnknown(SCPI_C, SCPI_P, Stream &) { err(-113, "Undefined header"); }

// ---- dispatch ----
// Collects everything the callbacks print so scpi_handle() can hand it to out().
class Sink : public Stream {
 public:
  char buf[1300];
  size_t n = 0;
  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
  void flush() override {}
  size_t write(uint8_t c) override {
    if (n < sizeof buf - 1) buf[n++] = c;
    return 1;
  }
};

static SCPI_Parser parser;
static Sink sink;

static void registerCommands() {
  static bool done;
  if (done) return;
  done = true;
  struct { const char *cmd; SCPI_caller_t fn; } cmds[] = {
      {"*IDN?", cIdn}, {"*RST", cRst}, {"*CLS", cCls}, {"*OPC", cOpc}, {"*OPC?", cOpc},
      {"*WAI", cNop}, {"*TRG", cNop}, {"*PSC", cNop}, {"*PSC?", cZero}, {"*TST?", cZero},
      {"*OPT?", cZero}, {"*ESE", cEse}, {"*ESE?", cEse}, {"*SRE", cSre}, {"*SRE?", cSre},
      {"*ESR?", cEsr}, {"*STB?", cStb},
      {"SYSTem:ERRor?", cErrNext}, {"SYSTem:ERRor:NEXT?", cErrNext}, {"SYSTem:ERRor:ALL?", cErrAll},
      {"SYSTem:PRESet", cPreset}, {"SYSTem:VERSion?", cVersion},
      {"DIAGnostic:SERVice:HWINfo?", cHwInfo},
      {"ROUTe:CLOSe", cClose}, {"ROUTe:CLOSe?", cClose}, {"ROUTe:CLOSe:COMBined", cCombined},
      {"ROUTe:PATH", cPathDef}, {"ROUTe:PATH?", cPathDef},
      {"ROUTe:PATH:DEFine", cPathDef}, {"ROUTe:PATH:DEFine?", cPathDef},
      {"ROUTe:PATH:DELete", cPathDel}, {"ROUTe:PATH:DELete:NAME", cPathDel},
      {"ROUTe:PATH:DELete:ALL", cPathDelAll}, {"ROUTe:PATH:CATalog?", cPathCat},
      {"ROUTe:PATH:LAST?", cPathLast}, {"ROUTe:PATH:MULTiple", cPathMult}, {"ROUTe:PATH:MULTiple?", cPathMult},
  };
  for (auto &c : cmds) parser.RegisterCommand(c.cmd, c.fn);
  parser.SetErrorHandler(cUnknown);
}

void scpi_handle(const char *line, Out out) {
  static char msg[512];
  registerCommands();
  strncpy(msg, line, sizeof msg - 1);  // Execute() modifies the message
  msg[sizeof msg - 1] = 0;
  sink.n = 0;
  parser.Execute(msg, sink);
  sink.buf[sink.n] = 0;
  if (sink.n) out(sink.buf);
}
