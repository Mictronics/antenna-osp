#!/bin/sh
# Stamp FW_VERSION from platformio.ini into the settings SVGs, then rebuild their PNG and .rgb565.
set -e
cd "$(dirname "$0")/.."
v=$(sed -n 's/.*-DFW_VERSION=\\"\(.*\)\\".*/\1/p' platformio.ini)
[ -n "$v" ] || { echo "FW_VERSION not found" >&2; exit 1; }
for t in dark hc light; do
  svg=screens/svg/settings-$t.svg
  sed -i "s|>v[0-9][0-9.]*</text>|>v$v</text>|" "$svg"
  [ $t = dark ] && n=settings || n=settings-$t
  inkscape "$svg" --export-type=png --export-filename="screens/$n.png" -w 480 -h 320 >/dev/null
  python3 tools/png2rgb565.py "screens/$n.png" "screens/$n.rgb565"
done
echo "stamped v$v"
