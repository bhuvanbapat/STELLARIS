"""Validates the provenance records of the observation assets (CLAUDE.md §19.7).

For every assets/observations/*.json sidecar: required fields are present and
non-empty, the referenced image exists, its SHA-256 matches the recorded checksum,
and its pixel dimensions (read from the JPEG header) match the record. Every image
in the folder must have a sidecar. Exit code 0 = all good.

usage:  python tools/check_observations.py [--facts]
        --facts  print checksum and dimensions of each image (for writing sidecars)
"""

import hashlib
import json
import os
import struct
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "observations")

REQUIRED = [
    "id", "title", "object", "stage", "pathway", "source_institution", "source_page", "file_url",
    "credit", "license", "observatory_instrument", "band", "band_kind", "image_kind",
    "processing_notes", "original_dimensions", "local_file", "local_dimensions",
    "derivative_operations", "intended_use", "sha256",
]
BAND_KINDS = {"visible", "infrared", "ultraviolet", "x-ray", "radio", "multi-band"}


def jpeg_size(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:2] != b"\xff\xd8":
        raise ValueError("not a JPEG")
    i = 2
    while i < len(data):
        if data[i] != 0xFF:
            i += 1
            continue
        marker = data[i + 1]
        if marker in (0xD8, 0x01) or 0xD0 <= marker <= 0xD7:
            i += 2
            continue
        length = struct.unpack(">H", data[i + 2:i + 4])[0]
        if marker in (0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF):
            h, w = struct.unpack(">HH", data[i + 5:i + 9])
            return w, h
        i += 2 + length
    raise ValueError("no SOF marker")


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    images = sorted(f for f in os.listdir(ROOT) if f.lower().endswith(".jpg"))
    if "--facts" in sys.argv:
        for f in images:
            p = os.path.join(ROOT, f)
            w, h = jpeg_size(p)
            print(f"{f}: {w}x{h} sha256={sha256(p)}")
        return 0

    errors = []
    sidecars = sorted(f for f in os.listdir(ROOT) if f.endswith(".json"))
    covered = set()
    for name in sidecars:
        with open(os.path.join(ROOT, name), encoding="utf-8") as f:
            rec = json.load(f)
        for key in REQUIRED:
            if not str(rec.get(key, "")).strip():
                errors.append(f"{name}: missing or empty '{key}'")
        if rec.get("band_kind") not in BAND_KINDS:
            errors.append(f"{name}: band_kind must be one of {sorted(BAND_KINDS)}")
        img = rec.get("local_file", "")
        path = os.path.join(ROOT, img)
        if not os.path.isfile(path):
            errors.append(f"{name}: image '{img}' not found")
            continue
        covered.add(img)
        if sha256(path) != rec.get("sha256"):
            errors.append(f"{name}: sha256 mismatch for {img}")
        w, h = jpeg_size(path)
        if rec.get("local_dimensions") != f"{w}x{h}":
            errors.append(f"{name}: local_dimensions {rec.get('local_dimensions')} != actual {w}x{h}")
    for img in images:
        if img not in covered:
            errors.append(f"{img}: no provenance sidecar")
    for e in errors:
        print("ERROR", e)
    print(f"{len(sidecars)} records, {len(images)} images, {len(errors)} problem(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
