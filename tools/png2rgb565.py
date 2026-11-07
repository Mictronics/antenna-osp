#!/usr/bin/env python3
"""Convert an image to 320x480 big-endian RGB565 for the SD card: png2rgb565.py in.png out.rgb565"""
import sys
from PIL import Image

img = Image.open(sys.argv[1]).convert("RGB").resize((320, 480))
out = bytearray()
for r, g, b in img.getdata():
    v = (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3
    out += v.to_bytes(2, "big")
open(sys.argv[2], "wb").write(out)
