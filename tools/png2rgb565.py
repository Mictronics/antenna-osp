#!/usr/bin/env python3
"""Convert an image to big-endian RGB565 for the SD card: png2rgb565.py in.png out.rgb565 [W H]  (default 480x320)"""
import sys
from PIL import Image

img = Image.open(sys.argv[1]).convert("RGB").resize((int(sys.argv[3]), int(sys.argv[4])) if len(sys.argv) > 4 else (480, 320))
out = bytearray()
for r, g, b in img.getdata():
    v = (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3
    out += v.to_bytes(2, "big")
open(sys.argv[2], "wb").write(out)
