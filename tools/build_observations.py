"""Builds Observation-view entries from the acquired reference library.

For every library image linked to a stage (assets/astronomy/metadata/manifest.json),
writes a display derivative (longest side <= 3072 px) to assets/observations/lib_<id>.jpg
and its provenance sidecar. Excluded from the Observation view:
  - artist's impressions / illustrations (they are not observations);
  - assets with no stage (e.g. Type Ia remnants: not part of a single-star pathway);
  - videos and data files.
Band classification is by observatory (Webb: infrared; Hubble: visible unless the title
says infrared; ESO: multi-band; SDO: by channel) and is stated as such in the sidecar.

usage: python tools/build_observations.py
"""
import hashlib
import json
import os

from PIL import Image

Image.MAX_IMAGE_PIXELS = None
HERE = os.path.dirname(os.path.abspath(__file__))
LIB = os.path.join(HERE, "..", "assets", "astronomy")
OUT = os.path.join(HERE, "..", "assets", "observations")
MAX_SIDE = 3072
NOT_OBSERVATION = ("artist", "impression", "illustration", "animation", "simulation", "infographic", "graphic")


def classify(rec):
    org, title = rec["organization"], rec["title"].lower()
    if org == "ESA/Webb":
        return ("James Webb Space Telescope" + (" (NIRCam)" if "nircam" in title else " (MIRI)" if "miri" in title else ""),
                "Infrared (Webb NIRCam/MIRI; see source page)", "infrared")
    if org == "ESA/Hubble":
        ir = "infrared" in title
        return ("Hubble Space Telescope", "Near-infrared" if ir else "Visible / near-UV-to-near-IR filters (see source page)",
                "infrared" if ir else "visible")
    if org == "ESO":
        return ("ESO facilities (VLT, VISTA, VST, ALMA, ...; see source page)", "See source page", "multi-band")
    if org in ("science.nasa.gov", "svs.gsfc.nasa.gov", "chandra.harvard.edu"):
        k = rec.get("kind", "").lower()
        bk = "infrared" if ("infrared" in k or "webb" in k) else "visible" if "visible" in k else              "x-ray" if "x-ray" in k else "multi-band"
        return (rec.get("kind", "NASA"), rec.get("kind", "see source page"), bk)
    band = rec.get("band", "")
    kind = "visible" if "HMI" in band or "1700" in band else "ultraviolet"
    return ("Solar Dynamics Observatory (" + band.split(" ")[0] + ")", band, kind)


def short_name(ident):
    """Stable, short file stem (keeps clone paths well under Windows' 260-char limit)."""
    import hashlib, re
    s = f"lib_{ident}"
    if len(s) <= 56:
        return s
    return "lib_" + re.sub(r"_+", "_", ident)[:40].strip("_") + "_" + hashlib.sha1(ident.encode()).hexdigest()[:8]


def main():
    manifest = json.load(open(os.path.join(LIB, "metadata", "manifest.json"), encoding="utf-8"))
    made = skipped = 0
    for ident, rec in sorted(manifest.items()):
        title = rec["title"]
        if not rec.get("stage") or not rec["local_file"].lower().endswith(".jpg") or \
                any(w in (title + " " + (rec.get("kind", "") if rec["organization"] in ("science.nasa.gov", "svs.gsfc.nasa.gov", "chandra.harvard.edu") else "")).lower() for w in NOT_OBSERVATION):
            skipped += 1
            continue
        if "vasquez" in ident:  # landscape photo (CC BY-NC) mis-filed under a nebula page
            skipped += 1
            continue
        stem = short_name(ident)
        src = os.path.join(LIB, rec["local_file"])
        dst_name = f"{stem}.jpg"
        dst = os.path.join(OUT, dst_name)
        if not os.path.exists(dst):
            with Image.open(src) as im:
                im.draft("RGB", (MAX_SIDE, MAX_SIDE))  # decode huge JPEGs at reduced scale
                im = im.convert("RGB")
                scale = min(1.0, MAX_SIDE / max(im.size))
                if scale < 1.0:
                    im = im.resize((round(im.width * scale), round(im.height * scale)), Image.LANCZOS)
                im.save(dst, "JPEG", quality=92, optimize=True)
        with Image.open(dst) as im:
            local_dims = f"{im.width}x{im.height}"
        instrument, band, band_kind = classify(rec)
        sidecar = {
            "id": stem, "title": title, "object": title, "stage": rec["stage"], "pathway": rec["pathway"],
            "source_institution": rec["organization"], "source_page": rec["source_page"], "file_url": rec["file_url"],
            "credit": rec.get("credit") or f"{rec['organization']} (full credit on the source page)",
            "license": rec["license"], "observatory_instrument": instrument, "band": band, "band_kind": band_kind,
            "image_kind": "processed colour image released by the observatory (see source page for filters)",
            "processing_notes": "Metadata compiled automatically from the source page by tools/acquire_assets.py; "
                                "band classified by observatory. " + rec.get("note", ""),
            "original_dimensions": rec.get("dimensions", ""), "local_file": dst_name, "local_dimensions": local_dims,
            "derivative_operations": f"resized to <= {MAX_SIDE} px on the longest side (Lanczos), JPEG quality 92; "
                                     f"original kept in assets/astronomy/{rec['local_file']}",
            "intended_use": "Observation view reference (library image) beside the conceptual STELLARIS scene",
            "retrieved": rec.get("retrieved", ""),
            "sha256": hashlib.sha256(open(dst, "rb").read()).hexdigest(),
        }
        json.dump(sidecar, open(os.path.join(OUT, f"{stem}.json"), "w", encoding="utf-8", newline="\n"),
                  indent=2, ensure_ascii=False)
        made += 1
    print(f"observation entries: {made} written, {skipped} skipped (no stage, not an image, or not an observation)")


if __name__ == "__main__":
    main()
