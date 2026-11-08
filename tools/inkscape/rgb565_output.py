#!/usr/bin/env python3
"""Inkscape "Save a Copy" format: render the page to 480x320 big-endian RGB565 (needs Pillow)."""
import os
import tempfile

import inkex
from inkex.command import inkscape
from PIL import Image


class Rgb565Output(inkex.OutputExtension):
    def save(self, stream):
        with tempfile.TemporaryDirectory() as d:
            svg, png = os.path.join(d, "in.svg"), os.path.join(d, "out.png")
            self.document.write(svg)
            inkscape(svg, export_type="png", export_filename=png,
                     export_width=480, export_height=320, export_area_page=True)
            img = Image.open(png).convert("RGB")
        out = bytearray()
        for r, g, b in img.getdata():
            out += ((r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3).to_bytes(2, "big")
        stream.write(out)


if __name__ == "__main__":
    Rgb565Output().run()
