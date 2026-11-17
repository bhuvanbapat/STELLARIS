"""STELLARIS astronomical reference acquisition.

Discovers images through the public search pages of ESA/Webb, ESA/Hubble and ESO,
keeps only results whose title names the requested object, downloads the largest
public JPEG of each (resumable, retried), and pulls full-resolution SDO images and
reference movies from NASA. Writes assets/astronomy/metadata/manifest.json with
title, credit, licence, source page, URL, size, dimensions and SHA-256 of each file,
and a log of every success and failure. Re-running skips completed files.

The library itself (several GB) is not committed; this script reproduces it.

usage: python tools/acquire_assets.py
"""
import hashlib
import html
import json
import os
import re
import subprocess
import sys
import time

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "astronomy")
META = os.path.join(ROOT, "metadata")

SITES = {
    "webb":   {"search": "https://esawebb.org/images/?search={q}", "page": "https://esawebb.org/images/{id}/",
               "large": "https://cdn.esawebb.org/archives/images/large/{id}.jpg",
               "licence": "CC BY 4.0 (https://esawebb.org/copyright/)", "org": "ESA/Webb"},
    "hubble": {"search": "https://esahubble.org/images/?search={q}", "page": "https://esahubble.org/images/{id}/",
               "large": "https://cdn.esahubble.org/archives/images/large/{id}.jpg",
               "licence": "CC BY 4.0 (https://esahubble.org/copyright/)", "org": "ESA/Hubble"},
    "eso":    {"search": "https://www.eso.org/public/images/?search={q}", "page": "https://www.eso.org/public/images/{id}/",
               "large": "https://cdn.eso.org/images/large/{id}.jpg",
               "licence": "CC BY 4.0 (https://www.eso.org/public/copyright/)", "org": "ESO"},
}

# (category, site, query, title keywords (any), max results, stage, pathway)
TARGETS = [
    ("molecular_clouds", "hubble", "Eagle Nebula", ["eagle", "pillars"], 2, "1-2", "all"),
    ("molecular_clouds", "webb", "Pillars of Creation", ["pillars"], 2, "1-2", "all"),
    ("molecular_clouds", "eso", "Eagle Nebula", ["eagle", "messier 16", "m16"], 1, "1-2", "all"),
    ("molecular_clouds", "webb", "Carina", ["carina", "cosmic cliffs"], 2, "1-2", "all"),
    ("molecular_clouds", "hubble", "Carina Nebula", ["carina"], 2, "1-2", "all"),
    ("molecular_clouds", "hubble", "Orion Nebula", ["orion"], 1, "1-2", "all"),
    ("molecular_clouds", "eso", "Orion VISTA", ["orion"], 1, "1-2", "all"),
    ("molecular_clouds", "webb", "Orion", ["orion"], 1, "1-2", "all"),
    ("molecular_clouds", "webb", "Serpens", ["serpens"], 1, "1-2", "all"),
    ("molecular_clouds", "webb", "NGC 1333", ["1333"], 1, "1-2", "all"),
    ("molecular_clouds", "hubble", "NGC 1333", ["1333"], 1, "1-2", "all"),
    ("molecular_clouds", "eso", "NGC 2264", ["2264", "cone"], 1, "1-2", "all"),
    ("molecular_clouds", "hubble", "Cone Nebula", ["cone"], 1, "1-2", "all"),
    ("molecular_clouds", "webb", "AFGL 5180", ["5180"], 1, "1-2", "all"),
    ("molecular_clouds", "hubble", "AFGL 5180", ["5180"], 1, "1-2", "all"),
    ("molecular_clouds", "hubble", "Westerlund 2", ["westerlund"], 1, "1-2", "all"),
    ("molecular_clouds", "webb", "Westerlund", ["westerlund"], 1, "1-2", "all"),
    ("protostars", "webb", "L1527", ["l1527", "protostar"], 1, "3-4", "all"),
    ("protostars", "webb", "Lynds 483", ["483"], 1, "3-4", "all"),
    ("protostars", "webb", "protostar", ["protostar"], 2, "3-4", "all"),
    ("accretion_disks", "eso", "HL Tau", ["hl tau"], 1, "3-4", "all"),
    ("accretion_disks", "eso", "protoplanetary disc ALMA", ["disc", "disk"], 2, "3-4", "all"),
    ("accretion_disks", "hubble", "HH 30", ["hh 30", "herbig-haro 30"], 1, "3-4", "all"),
    ("accretion_disks", "webb", "HH 30", ["hh 30"], 1, "3-4", "all"),
    ("bipolar_jets", "webb", "HH 211", ["211"], 1, "3-4", "all"),
    ("bipolar_jets", "webb", "HH 46/47", ["46", "47"], 1, "3-4", "all"),
    ("bipolar_jets", "webb", "HH 49", ["49"], 1, "3-4", "all"),
    ("herbig_haro_objects", "webb", "HH 24", ["hh 24", "herbig-haro 24"], 1, "3-4", "all"),
    ("herbig_haro_objects", "hubble", "HH 24", ["hh 24", "herbig-haro 24"], 1, "3-4", "all"),
    ("herbig_haro_objects", "hubble", "Herbig-Haro", ["herbig", "hh "], 3, "3-4", "all"),
    ("herbig_haro_objects", "eso", "Herbig-Haro", ["herbig", "hh "], 1, "3-4", "all"),
    ("red_giants", "eso", "Betelgeuse", ["betelgeuse"], 2, "6", "all"),
    ("red_giants", "eso", "Pi1 Gruis", ["gruis"], 1, "6", "all"),
    ("red_giants", "eso", "R Doradus", ["doradus"], 1, "6", "all"),
    ("red_giants", "eso", "WOH G64", ["woh"], 1, "6", "all"),
    ("red_giants", "eso", "red giant star surface", ["surface", "giant"], 1, "6", "all"),
    ("planetary_nebulae", "eso", "Helix Nebula", ["helix"], 1, "6", "white-dwarf"),
    ("planetary_nebulae", "hubble", "Helix", ["helix"], 1, "6", "white-dwarf"),
    ("planetary_nebulae", "webb", "Ring Nebula", ["ring nebula"], 2, "6", "white-dwarf"),
    ("planetary_nebulae", "hubble", "Cat's Eye Nebula", ["cat"], 1, "6", "white-dwarf"),
    ("planetary_nebulae", "hubble", "Butterfly Nebula", ["butterfly"], 1, "6", "white-dwarf"),
    ("planetary_nebulae", "webb", "NGC 6302", ["6302", "butterfly"], 1, "6", "white-dwarf"),
    ("planetary_nebulae", "eso", "Dumbbell Nebula", ["dumbbell", "messier 27"], 1, "6", "white-dwarf"),
    ("planetary_nebulae", "hubble", "Eskimo Nebula", ["eskimo"], 1, "6", "white-dwarf"),
    ("white_dwarfs", "hubble", "Sirius B", ["sirius"], 1, "6", "white-dwarf"),
    ("supernova_progenitors", "hubble", "Eta Carinae", ["eta carinae"], 1, "6", "neutron-star"),
    ("supernova_progenitors", "eso", "Wolf-Rayet", ["wolf-rayet", "wr "], 1, "6", "black-hole"),
    ("supernova_explosions", "hubble", "SN 1987A", ["1987a"], 2, "6", "neutron-star"),
    ("supernova_explosions", "webb", "SN 1987A", ["1987a"], 1, "6", "neutron-star"),
    ("supernova_explosions", "eso", "SN 1987A", ["1987a"], 1, "6", "neutron-star"),
    ("supernova_remnants", "webb", "Cassiopeia A", ["cassiopeia"], 1, "6", "neutron-star"),
    ("supernova_remnants", "hubble", "Cassiopeia A", ["cassiopeia"], 1, "6", "neutron-star"),
    ("supernova_remnants", "webb", "Crab Nebula", ["crab"], 1, "6", "neutron-star"),
    ("supernova_remnants", "hubble", "Crab Nebula", ["crab"], 1, "6", "neutron-star"),
    ("supernova_remnants", "hubble", "Tycho supernova", ["tycho"], 1, "", "type-ia"),
    ("supernova_remnants", "hubble", "Kepler supernova remnant", ["kepler"], 1, "", "type-ia"),
    ("supernova_remnants", "eso", "supernova remnant", ["remnant"], 1, "6", "neutron-star"),
    ("neutron_stars", "eso", "neutron star", ["neutron", "pulsar", "magnetar"], 1, "6", "neutron-star"),
    ("neutron_stars", "hubble", "Crab pulsar", ["pulsar"], 1, "6", "neutron-star"),
]

SDO = "https://sdo.gsfc.nasa.gov/assets/img/latest/"
SDO_IMAGES = [  # (file, band, kind) full-disk 4096x4096, public domain (NASA)
    ("latest_4096_HMIIC.jpg", "HMI continuum, 617.3 nm (visible)", "solar_photosphere"),
    ("latest_4096_HMIIF.jpg", "HMI flattened continuum, 617.3 nm (visible)", "solar_photosphere"),
    ("latest_4096_HMIB.jpg", "HMI line-of-sight magnetogram (Fe I 617.3 nm)", "solar_photosphere"),
    ("latest_4096_1700.jpg", "AIA 1700 A (UV continuum: photosphere)", "solar_chromosphere"),
    ("latest_4096_1600.jpg", "AIA 1600 A (UV: upper photosphere / transition region)", "solar_chromosphere"),
    ("latest_4096_0304.jpg", "AIA 304 A (EUV, He II: chromosphere, prominences)", "solar_prominences"),
    ("latest_4096_0171.jpg", "AIA 171 A (EUV, Fe IX: quiet corona, coronal loops)", "solar_corona"),
    ("latest_4096_0193.jpg", "AIA 193 A (EUV, Fe XII: corona, hot flare plasma)", "solar_corona"),
    ("latest_4096_0211.jpg", "AIA 211 A (EUV, Fe XIV: active-region corona)", "solar_corona"),
    ("latest_4096_0131.jpg", "AIA 131 A (EUV, Fe VIII/XXI: flaring regions)", "solar_corona"),
    ("latest_4096_0094.jpg", "AIA 94 A (EUV, Fe XVIII: flaring regions)", "solar_corona"),
]
SDO_MOVIES = [  # 48-hour reference movies, public domain (NASA)
    ("mpeg/latest_1024_0171.mp4", "AIA 171 A, last 48 h"),
    ("mpeg/latest_1024_0304.mp4", "AIA 304 A, last 48 h"),
    ("mpeg/latest_1024_0193.mp4", "AIA 193 A, last 48 h"),
    ("mpeg/latest_1024_HMIIC.mp4", "HMI continuum, last 48 h"),
    ("mpeg/latest_1024_HMIB.mp4", "HMI magnetogram, last 48 h"),
]

UA = ["-A", "STELLARIS-asset-acquisition/1.0 (educational project)"]


def curl_text(url):
    r = subprocess.run(["curl", "-sL", "--max-time", "60", *UA, url], capture_output=True)
    return r.stdout.decode("utf-8", "replace")


def download(url, path):
    """Resumable download with retries; returns (ok, message)."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    part = path + ".part"
    if os.path.exists(path):
        return True, "already present"
    for attempt in range(4):
        r = subprocess.run(["curl", "-sSL", "--fail", "-C", "-", "--retry", "3", "--max-time", "1800", *UA,
                            "-o", part, url], capture_output=True)
        if r.returncode == 0:
            os.replace(part, path)
            return True, "downloaded"
        if r.returncode == 22:  # HTTP error (e.g. 404): no point retrying
            break
        time.sleep(3)
    if os.path.exists(part) and os.path.getsize(part) == 0:
        os.remove(part)
    return False, (r.stderr.decode("utf-8", "replace").strip() or f"curl exit {r.returncode}")[:200]


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def dims(path):
    try:
        from PIL import Image
        Image.MAX_IMAGE_PIXELS = None
        with Image.open(path) as im:
            return f"{im.width}x{im.height}"
    except Exception:
        return ""


def search(site, query):
    text = curl_text(SITES[site]["search"].format(q=query.replace(" ", "+")))
    out = []
    for m in re.finditer(r"id: '([a-z0-9-]+)',\s*title: '([^']*)'", text):
        out.append((m.group(1), html.unescape(m.group(2))))
    if not out:  # ESA/Webb markup
        for m in re.finditer(r'href="/images/([a-z0-9-]+)/" aria-label="([^"]*)"', text):
            out.append((m.group(1), html.unescape(m.group(2))))
    seen, uniq = set(), []
    for i, t in out:
        if i not in seen and i not in ("potm", "potw", "viewall", "search"):
            seen.add(i); uniq.append((i, t))
    return uniq


def page_credit(site, ident):
    text = curl_text(SITES[site]["page"].format(id=ident))
    m = re.search(r'class="credit"[^>]*>(.*?)</div>', text, re.S)
    credit = re.sub(r"<[^>]+>", " ", m.group(1)) if m else ""
    credit = re.sub(r"\s+", " ", html.unescape(credit)).strip()
    return credit


def main():
    os.makedirs(META, exist_ok=True)
    manifest_path = os.path.join(META, "manifest.json")
    manifest = json.load(open(manifest_path, encoding="utf-8")) if os.path.exists(manifest_path) else {}
    log = []
    taken = set(manifest)
    for cat, site, query, keys, nmax, stage, pathway in TARGETS:
        try:
            results = search(site, query)
        except Exception as e:  # network failure: logged, run continues
            log.append(("FAILED", cat, site, query, f"search error: {e}"))
            continue
        picked = [(i, t) for i, t in results if any(k in t.lower() for k in keys)][:nmax]
        if not picked:
            log.append(("UNAVAILABLE", cat, site, query, f"no result titled with {keys} ({len(results)} results)"))
            continue
        for ident, title in picked:
            if ident in taken:
                log.append(("SKIPPED", cat, site, ident, "already acquired under another query"))
                continue
            url = SITES[site]["large"].format(id=ident)
            path = os.path.join(ROOT, cat, f"{ident}.jpg")
            ok, msg = download(url, path)
            if not ok:
                log.append(("FAILED", cat, site, ident, msg)); continue
            taken.add(ident)
            manifest[ident] = {
                "id": ident, "title": title, "category": cat, "query": query, "organization": SITES[site]["org"],
                "source_page": SITES[site]["page"].format(id=ident), "file_url": url,
                "credit": page_credit(site, ident), "license": SITES[site]["licence"],
                "local_file": os.path.relpath(path, ROOT).replace("\\", "/"), "bytes": os.path.getsize(path),
                "dimensions": dims(path), "sha256": sha256(path), "kind": "observation (see title: some are artist's impressions or composites)",
                "stage": stage, "pathway": pathway, "retrieved": time.strftime("%Y-%m-%d")}
            log.append(("OK", cat, site, ident, f"{title} | {manifest[ident]['dimensions']} | {manifest[ident]['bytes'] / 2**20:.1f} MiB"))
            json.dump(manifest, open(manifest_path, "w", encoding="utf-8"), indent=2, ensure_ascii=False)
    for fname, band, cat in SDO_IMAGES:
        ident = "sdo_" + fname[len("latest_4096_"):-4]
        path = os.path.join(ROOT, cat, f"{ident}.jpg")
        ok, msg = download(SDO + fname, path)
        if not ok:
            log.append(("FAILED", cat, "sdo", ident, msg)); continue
        manifest[ident] = {
            "id": ident, "title": f"Sun, full disk: {band}", "category": cat, "organization": "NASA/SDO",
            "source_page": "https://sdo.gsfc.nasa.gov/data/", "file_url": SDO + fname,
            "credit": "Courtesy of NASA/SDO and the AIA, EVE, and HMI science teams.",
            "license": "Public domain (NASA); SDO data policy: free use with courtesy credit",
            "local_file": os.path.relpath(path, ROOT).replace("\\", "/"), "bytes": os.path.getsize(path),
            "dimensions": dims(path), "sha256": sha256(path), "kind": "observation", "band": band,
            "stage": "5", "pathway": "all", "retrieved": time.strftime("%Y-%m-%d"),
            "note": "'latest' image at retrieval time; EUV bands are colour-mapped, not visible light"}
        log.append(("OK", cat, "sdo", ident, f"{band} | {manifest[ident]['dimensions']}"))
    for fname, band in SDO_MOVIES:
        ident = "sdo_movie_" + os.path.basename(fname)[len("latest_1024_"):-4]
        path = os.path.join(ROOT, "reference_videos", f"{ident}.mp4")
        ok, msg = download(SDO + fname, path)
        if not ok:
            log.append(("FAILED", "reference_videos", "sdo", ident, msg)); continue
        manifest[ident] = {
            "id": ident, "title": f"Sun: {band}", "category": "reference_videos", "organization": "NASA/SDO",
            "source_page": "https://sdo.gsfc.nasa.gov/data/", "file_url": SDO + fname,
            "credit": "Courtesy of NASA/SDO and the AIA, EVE, and HMI science teams.",
            "license": "Public domain (NASA)", "local_file": os.path.relpath(path, ROOT).replace("\\", "/"),
            "bytes": os.path.getsize(path), "dimensions": "1024x1024 video", "sha256": sha256(path),
            "kind": "observation (time-lapse)", "stage": "5", "pathway": "all", "retrieved": time.strftime("%Y-%m-%d")}
        log.append(("OK", "reference_videos", "sdo", ident, band))
    json.dump(manifest, open(manifest_path, "w", encoding="utf-8"), indent=2, ensure_ascii=False)
    with open(os.path.join(META, "download_log.tsv"), "a", encoding="utf-8") as f:
        for row in log:
            f.write(time.strftime("%Y-%m-%d %H:%M") + "\t" + "\t".join(str(c) for c in row) + "\n")
    counts = {}
    for row in log:
        counts[row[0]] = counts.get(row[0], 0) + 1
    print("done", counts, "manifest entries:", len(manifest))
    return 0


if __name__ == "__main__":
    sys.exit(main())
