"""STELLARIS reference acquisition, part 2: NASA SVS, science.nasa.gov and Chandra media.

For each page: the highest-resolution H.264/MP4 video (skipping files > 4 GB, logged with
their size), up to 4 large stills (PNG/TIFF/JPEG, no thumbnails), and for SVS frame
directories an evenly spaced subset of original frames (not tens of thousands of
files). Appends to assets/astronomy/metadata/manifest.json and the download log.

usage: python tools/acquire_media.py
"""
import json
import os
import re
import subprocess
import sys
import time
import urllib.parse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from acquire_assets import ROOT, META, download, sha256, dims, curl_text, UA  # noqa: E402

MAX_BYTES = 4 * 2**30
FRAMES_PER_SEQUENCE = 48

# (category, page, kind, stage, pathway, note)
PAGES = [
    ("reference_frame_sequences", "https://svs.gsfc.nasa.gov/5649/", "observation (SDO/AIA time series)", "5", "all",
     "Four days of SDO: 171 A corona and 304 A chromosphere, 4096x4096 frames (EUV, colour-mapped)"),
    ("reference_videos", "https://svs.gsfc.nasa.gov/4776/", "observation (SDO time-lapse)", "5", "all",
     "Ten years of SDO (solar cycle)"),
    ("molecular_clouds", "https://svs.gsfc.nasa.gov/31372/", "visualization of an observation (Webb image, 3D fly-through)", "1-2", "all",
     "Serpens Nebula tour: aligned protostellar outflows"),
    ("bipolar_jets", "https://svs.gsfc.nasa.gov/31344/", "visualization of an observation (Webb image)", "3-4", "all",
     "Herbig-Haro 49/50 bow shock"),
    ("supernova_explosions", "https://svs.gsfc.nasa.gov/20413/", "artistic visualization (animation)", "", "core-collapse",
     "Supernova explosion animation: reference for timing and scale only"),
    ("scientific_simulations", "https://svs.gsfc.nasa.gov/20344/", "artistic visualization (animation)", "", "type-ia",
     "Type Ia supernova animations: binary white dwarf; NOT part of the single-star pathways"),
    ("supernova_remnants", "https://svs.gsfc.nasa.gov/10547/", "artistic visualization (animation)", "", "core-collapse",
     "Expanding supernova-remnant shell animation"),
    ("bipolar_jets", "https://science.nasa.gov/asset/webb/hh-211-nircam-image/", "observation (Webb NIRCam, infrared)", "3-4", "all",
     "HH 211 full resolution: H2/CO/SiO shock emission (infrared, colour-mapped)"),
    ("scientific_simulations", "https://science.nasa.gov/asset/hubble/blast-wave-from-a-stellar-explosion-simulation-of-supernova-1987a/",
     "simulation-based visualization", "6", "neutron-star", "SN 1987A blast wave hitting the circumstellar ring"),
    ("molecular_clouds", "https://science.nasa.gov/asset/hubble/pillars-of-creation-visible-and-infrared-views/",
     "observation (Hubble visible and near-infrared)", "1-2", "all", "Pillars of Creation, visible vs infrared"),
    ("planetary_nebulae", "https://science.nasa.gov/asset/hubble/hubbles-planetary-nebula-gallery/",
     "observation (Hubble)", "6", "white-dwarf", "Planetary nebula gallery: shells, bipolar lobes, filaments"),
    ("supernova_remnants", "https://chandra.harvard.edu/resources/animations/snr.html", "animations (Chandra)", "", "mixed",
     "Chandra supernova-remnant animations: reference only"),
    ("supernova_remnants", "https://chandra.harvard.edu/photo/2026/kepler/animations.html", "observation time-lapse (Chandra X-ray, 2000-2025)",
     "", "type-ia", "Kepler remnant expansion: Type Ia; reference only"),
]

MEDIA_RE = re.compile(r'(?:href|src)="([^"]+?\.(?:mp4|mov|m4v|png|tif|tiff|jpg|jpeg))"', re.I)


def head_size(url):
    r = subprocess.run(["curl", "-sIL", "--max-time", "60", *UA, url], capture_output=True)
    sizes = re.findall(rb"(?i)content-length:\s*(\d+)", r.stdout)
    return int(sizes[-1]) if sizes else -1


def score_video(u):
    s = u.lower()
    for k, v in (("8k", 8), ("4320", 8), ("4k", 6), ("4096", 6), ("3840", 6), ("2160", 6), ("2048", 4),
                 ("1920", 3), ("1080", 3), ("1024", 2)):
        if k in s:
            return v - (0.5 if "stamped" in s else 0) - (5 if "prores" in s or s.endswith(".mov") else 0)
    return 1


def frames_subset(page_url, frames_dir):
    listing = curl_text(urllib.parse.urljoin(page_url, frames_dir))
    names = sorted(set(re.findall(r'href="([^"/?]+\.(?:png|jpg|tif))"', listing, re.I)))
    if not names:
        return []
    step = max(1, len(names) // FRAMES_PER_SEQUENCE)
    return [urllib.parse.urljoin(urllib.parse.urljoin(page_url, frames_dir), n) for n in names[::step][:FRAMES_PER_SEQUENCE]], len(names)


def main():
    manifest_path = os.path.join(META, "manifest.json")
    manifest = json.load(open(manifest_path, encoding="utf-8"))
    log = []

    def record(ident, cat, url, path, title, page, kind, stage, pathway, note, extra=None):
        manifest[ident] = {
            "id": ident, "title": title, "category": cat, "organization": urllib.parse.urlparse(page).netloc,
            "source_page": page, "file_url": url, "credit": "NASA (see source page for the full credit)",
            "license": "NASA media: generally not copyrighted (https://www.nasa.gov/nasa-brand-center/images-and-media/); "
                       "check the page for third-party elements",
            "local_file": os.path.relpath(path, ROOT).replace("\\", "/"), "bytes": os.path.getsize(path),
            "dimensions": dims(path) if not path.lower().endswith((".mp4", ".mov", ".m4v")) else "video",
            "sha256": sha256(path), "kind": kind, "stage": stage, "pathway": pathway, "note": note,
            "retrieved": time.strftime("%Y-%m-%d"), **(extra or {})}

    for cat, page, kind, stage, pathway, note in PAGES:
        text = curl_text(page)
        if not text:
            log.append(("FAILED", cat, "page", page, "page not reachable")); continue
        tag = re.sub(r"[^a-z0-9]+", "_", urllib.parse.urlparse(page).path.strip("/").lower())[-40:]
        links = sorted(set(urllib.parse.urljoin(page, u) for u in MEDIA_RE.findall(text)))
        links = [u for u in links if "favicon" not in u and "/logo" not in u.lower() and not u.endswith(".mp4.png")
                 and "thumb" not in u.lower() and "-150x" not in u and "-300x" not in u and "-768x" not in u]
        videos = sorted([u for u in links if u.lower().endswith((".mp4", ".mov", ".m4v"))], key=score_video, reverse=True)
        stills = [u for u in links if not u.lower().endswith((".mp4", ".mov", ".m4v"))]
        got = 0
        for v in videos[:3]:
            size = head_size(v)
            if size > MAX_BYTES:
                log.append(("SKIPPED", cat, "size", v, f"{size / 2**30:.1f} GiB > 4 GiB")); continue
            path = os.path.join(ROOT, cat if cat != "reference_frame_sequences" else "reference_videos",
                                f"{tag}__{os.path.basename(urllib.parse.urlparse(v).path)}")
            ok, msg = download(v, path)
            if ok:
                record(f"{tag}__{os.path.basename(path)}", cat, v, path, f"{note} (video)", page, kind, "", pathway, note)
                log.append(("OK", cat, "video", v, f"{os.path.getsize(path) / 2**20:.0f} MiB")); got += 1
                break
            log.append(("FAILED", cat, "video", v, msg))
        for s in stills[:4]:
            path = os.path.join(ROOT, cat if cat != "reference_frame_sequences" else "solar_corona",
                                f"{tag}__{os.path.basename(urllib.parse.urlparse(s).path)}")
            ok, msg = download(s, path)
            if ok and os.path.getsize(path) > 200_000:
                record(f"{tag}__{os.path.basename(path)}", cat, s, path, note, page, kind, stage, pathway, note)
                log.append(("OK", cat, "still", s, dims(path))); got += 1
            elif ok:
                os.remove(path)  # small preview image: not useful
            else:
                log.append(("FAILED", cat, "still", s, msg))
        for fdir in sorted(set(re.findall(r'href="([^"]*/frames/[^"]+/)"', text))):
            if "stamped" in fdir.lower():
                continue  # keep the unannotated frames
            res = frames_subset(page, fdir)
            if not res:
                log.append(("UNAVAILABLE", cat, "frames", fdir, "directory listing not readable")); continue
            urls, total = res
            seq = re.sub(r"[^A-Za-z0-9]+", "_", fdir.strip("/").split("/")[-1])
            outdir = os.path.join(ROOT, "reference_frame_sequences", f"{tag}__{seq}")
            ok_frames = sum(1 for u in urls if download(u, os.path.join(outdir, os.path.basename(u)))[0])
            ident = f"{tag}__{seq}"
            manifest[ident] = {
                "id": ident, "title": f"{note}: frame sequence {seq}", "category": "reference_frame_sequences",
                "organization": "svs.gsfc.nasa.gov", "source_page": page, "file_url": urllib.parse.urljoin(page, fdir),
                "credit": "NASA/SDO and the AIA science team; NASA Scientific Visualization Studio",
                "license": "NASA media: generally not copyrighted", "local_file": os.path.relpath(outdir, ROOT).replace("\\", "/"),
                "bytes": sum(os.path.getsize(os.path.join(outdir, f)) for f in os.listdir(outdir)),
                "dimensions": f"{ok_frames} of {total} frames (every {max(1, total // FRAMES_PER_SEQUENCE)}th), 4096x4096",
                "sha256": "per-frame (directory)", "kind": kind, "stage": "", "pathway": pathway, "note": note,
                "frames": ok_frames, "retrieved": time.strftime("%Y-%m-%d")}
            log.append(("OK", "reference_frame_sequences", "frames", fdir, f"{ok_frames}/{total} frames"))
            got += 1
        if not got:
            log.append(("UNAVAILABLE", cat, "page", page, f"no downloadable media found ({len(links)} links)"))
        json.dump(manifest, open(manifest_path, "w", encoding="utf-8"), indent=2, ensure_ascii=False)
    with open(os.path.join(META, "download_log.tsv"), "a", encoding="utf-8") as f:
        for row in log:
            f.write(time.strftime("%Y-%m-%d %H:%M") + "\t" + "\t".join(str(c) for c in row) + "\n")
    counts = {}
    for row in log:
        counts[row[0]] = counts.get(row[0], 0) + 1
    print("done", counts, "manifest entries:", len(manifest))


if __name__ == "__main__":
    main()
