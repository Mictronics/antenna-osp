#!/usr/bin/env python3
"""RS485 smoke test: rs485_test.py [port] (default /dev/ttyUSB0), 115200 8N1.
Checks SCPI replies ("" = no reply) and pauses so you can look at relays and the UI."""
import sys
import serial

s = serial.Serial(sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0", 115200, timeout=0.5)
fails = 0


def ask(line):
    s.reset_input_buffer()
    s.write(line.encode() + b"\n")
    return s.read_until(b"\n").decode(errors="replace")


def check(line, want, prefix=False):
    global fails
    got = ask(line)
    ok = got.startswith(want) if prefix else got == want
    fails += not ok
    print(f"{'ok  ' if ok else 'FAIL'} {line!r} -> {got!r}" + ("" if ok else f" want {want!r}"))


def look(msg):
    input(f"  >> {msg} [Enter]")


check("*IDN?", "DIY,OSP-EMU,000001,", prefix=True)
check("SYST:ERR?", '0,"No error"\n')
check("DIAG:SERV:HWIN?", '"F01|OSP230|', prefix=True)
look("UI should now show the REMOTE screen")

# K12 only = route IN2
check("ROUT:CLOS (@F01M01(0112))", "")
check("ROUT:CLOS? (@F01M01(0112))", "1\n")
look("only relay K2 energised?")
check("*RST", "")  # ROUT:CLOS is additive; start clean
check("ROUT:CLOS (@F01M01(0111,0113,0115))", "")  # K1 K3 K5 = route IN6
look("K1 K3 K5 energised?")
check("*RST", "")
look("all relays off?")

# paths
check('ROUT:PATH:DEF "A",(@F01M01(0111,0114))', "")
check("ROUT:PATH:CAT?", '"A"\n')
check('ROUT:CLOS "A"', "")
check("ROUT:PATH:LAST?", '"A"\n')
look("K1 and K4 energised (route IN4)?")
check("ROUT:PATH:DEL:ALL", "")

# errors
check("FOO:BAR", "")
check("SYST:ERR?", '-113,"Undefined header"\n')

# theme (persists on SD, so restore normal at the end)
check("DISP:THEM HCON", "")
check("DISP:THEM?", "HCON\n")
look("REMOTE screen now high contrast?")
check("DISP:THEM NORM", "")
check("DISP:THEM?", "NORM\n")
look("REMOTE screen back to normal?")
check("DISP:THEM foo", "")
check("SYST:ERR?", '-224,"Illegal parameter value"\n')

# unlock
check("SYST:LOC", "")
look("UI should be back on the main screen; touch a route button. Then touch the REMOTE screen "
     "after the next command to confirm touch unlocks")
check("*OPC?", "1\n")
look("REMOTE screen shown again? touch it -> back to local?")
check("*RST", "")

print("FAILED:", fails if fails else "none")
sys.exit(bool(fails))
