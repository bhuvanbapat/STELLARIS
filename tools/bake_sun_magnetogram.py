"""Bakes a real SDO/HMI synoptic magnetogram into the star's surface-activity map.

Input : hmi.Synoptic_Mr.<CR>.fits (JSOC, http://jsoc.stanford.edu/data/hmi/synoptic/):
        radial photospheric field Br over a whole Carrington rotation, 3600 x 1440,
        cylindrical equal-area (x: Carrington longitude, 0.1 deg; y: sine latitude).
Output: <out>.png  2048 x 1024 equirectangular RGBA8:
          R = log-scaled |Br| (0 .. 3000 G), G = polarity (1 = positive, 0 = negative),
          B = 0, A = 255. Longitude is rolled so the strongest active region sits at
          the longitude where STELLARIS stages its flare.
        Prints the strongest active-region centres (lat, lon in the rolled frame),
        used as the anchors in app/shaders/stellar_activity.glsl.

usage: python bake_sun_magnetogram.py <fits> <out.png> [flare_lon_rad]
"""
import math
import sys

import numpy as np
from PIL import Image


def read_fits(path):
    raw = open(path, "rb").read()
    keys, off = {}, 0
    while True:
        card = raw[off:off + 80].decode("ascii", "replace"); off += 80
        k = card[:8].strip()
        if "=" in card[8:10]:
            keys[k] = card[10:].split("/")[0].strip().strip("'").strip()
        if k == "END":
            break
    off = ((off + 2879) // 2880) * 2880
    nx, ny = int(keys["NAXIS1"]), int(keys["NAXIS2"])
    assert keys["BITPIX"] == "-32"
    data = np.frombuffer(raw, dtype=">f4", count=nx * ny, offset=off).reshape(ny, nx).astype(np.float32)
    return np.nan_to_num(data), keys


def main():
    src, out = sys.argv[1], sys.argv[2]
    flare_lon = float(sys.argv[3]) if len(sys.argv) > 3 else 2.0
    br, keys = read_fits(src)          # rows: sine latitude from -1 (row 0) to +1
    ny, nx = br.shape
    W, H = 2048, 1024
    # Equirectangular target: row 0 = +90 deg (top), column 0 = longitude 0.
    lat = (0.5 - (np.arange(H) + 0.5) / H) * math.pi
    sinlat = np.sin(lat)
    rows = np.clip(((sinlat + 1.0) * 0.5 * ny).astype(int), 0, ny - 1)
    # CDELT1 < 0: column 0 is the largest Carrington longitude.
    lon = (np.arange(W) + 0.5) / W * 2.0 * math.pi
    cols = np.clip(((2.0 * math.pi - lon) / (2.0 * math.pi) * nx).astype(int), 0, nx - 1)
    m = br[rows][:, cols]
    # Strongest active regions: smooth |B| and pick separated maxima.
    a = np.abs(m)
    k = 9
    pad = np.pad(a, ((k, k), (k, k)), mode="wrap")
    c = np.cumsum(np.cumsum(pad, 0), 1)
    sm = (c[2 * k:, 2 * k:] - c[:-2 * k, 2 * k:] - c[2 * k:, :-2 * k] + c[:-2 * k, :-2 * k])[:H, :W] / (2 * k) ** 2
    peaks, s2 = [], sm.copy()
    for _ in range(5):
        y, x = np.unravel_index(np.argmax(s2), s2.shape)
        peaks.append((y, x, float(s2[y, x])))
        yy, xx = np.ogrid[:H, :W]
        dx = np.minimum(np.abs(xx - x), W - np.abs(xx - x))
        s2[(dx ** 2 + (yy - y) ** 2) < 90 ** 2] = 0
    # Roll so the strongest region sits at the flare longitude.
    shift = int(round((flare_lon / (2 * math.pi)) * W)) - peaks[0][1]
    m = np.roll(m, shift, axis=1)
    mag = np.clip(np.log10(1.0 + np.abs(m)) / math.log10(3001.0), 0.0, 1.0)
    pol = (m > 0).astype(np.float32)
    rgba = np.stack([mag * 255, pol * 255, np.zeros_like(mag), np.full_like(mag, 255)], -1).astype(np.uint8)
    Image.fromarray(rgba, "RGBA").save(out, optimize=True)
    print(f"CR {keys.get('CAR_ROT')} {keys.get('T_START')} .. {keys.get('T_STOP')} -> {out} ({W}x{H})")
    print(f"max |Br| {np.abs(br).max():.0f} G")
    for y, x, v in peaks:
        lat_r = (0.5 - (y + 0.5) / H) * math.pi
        lon_r = ((x + shift) % W + 0.5) / W * 2 * math.pi
        print(f"active region: lat {lat_r:+.3f} rad, lon {lon_r:.3f} rad, mean |B| {v:.0f} G")


if __name__ == "__main__":
    main()
