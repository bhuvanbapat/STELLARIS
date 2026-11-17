"""Bakes the Cassiopeia A 3D reconstruction into a 3D emission texture.

Input : assets/data/casa/CasA_supernova_remnant-ascii_vtks.zip (NASA/CXC/SAO,
        T. Delaney et al. 2010, ApJ 725, 2038: 3D structure of Cas A from Chandra
        X-ray, Spitzer infrared and ground-based optical Doppler data). The VTK files
        are iso-surfaces of the emission of each component.
Output: <out_dir>/casa_volume.raw  N^3 RGBA8, x fastest
        <out_dir>/casa_volume.json size, channel meaning, scale

Channels: R = silicon (Chandra Si, incl. HETG), G = iron (Chandra Fe-K),
          B = infrared argon/neon (Spitzer), A = fast optical knots + Si jets.
Method: every triangle is sampled densely and splatted into the grid, then blurred
(three box passes ~ Gaussian, sigma ~1.4 voxels) and normalized per channel. The
surfaces become thin emitting sheets: a rendering of the published iso-surfaces, not
a reconstruction of the full emissivity. The grid is centred on the central compact
object (the neutron star), which is where STELLARIS places its remnant.

usage: python bake_casa.py <zip> <out_dir> [N]
"""
import io
import json
import os
import sys
import zipfile

import numpy as np

COMPONENTS = {
    "newsi-ascii.vtk": 0, "newhetg-ascii.vtk": 0,
    "fekcorr-ascii.vtk": 1,
    "newar-ascii.vtk": 2,
    "newopt-ascii.vtk": 3, "newjets-ascii.vtk": 3,
}


def read_vtk(text):
    tok = text.split()
    i = tok.index("POINTS")
    n = int(tok[i + 1])
    pts = np.array(tok[i + 3:i + 3 + 3 * n], dtype=np.float64).reshape(n, 3)
    j = tok.index("TRIANGLE_STRIPS")
    nstrips = int(tok[j + 1])
    k = j + 3
    tris = []
    for _ in range(nstrips):
        m = int(tok[k]); idx = [int(v) for v in tok[k + 1:k + 1 + m]]; k += 1 + m
        for a in range(m - 2):
            tris.append((idx[a], idx[a + 1], idx[a + 2]))
    return pts, np.array(tris, dtype=np.int64)


def box_blur(v, r, axis):
    c = np.cumsum(np.pad(v, [(r + 1, r) if ax == axis else (0, 0) for ax in range(v.ndim)]), axis=axis)
    hi = np.take(c, np.arange(2 * r + 1, c.shape[axis]), axis=axis)
    lo = np.take(c, np.arange(0, c.shape[axis] - 2 * r - 1), axis=axis)
    return (hi - lo) / (2 * r + 1)


def main():
    src, out_dir = sys.argv[1], sys.argv[2]
    N = int(sys.argv[3]) if len(sys.argv) > 3 else 192
    z = zipfile.ZipFile(src)
    files = {os.path.basename(n): n for n in z.namelist() if n.endswith(".vtk") and "__MACOSX" not in n}
    cco, _ = read_vtk(z.read(files["cco-ascii.vtk"]).decode("ascii"))
    centre = cco.mean(axis=0)
    meshes = []
    for name, ch in COMPONENTS.items():
        pts, tris = read_vtk(z.read(files[name]).decode("ascii"))
        meshes.append((ch, pts - centre, tris))
    rmax = max(np.linalg.norm(p, axis=1).max() for _, p, _ in meshes)
    scale = (N * 0.47) / rmax                      # data units -> voxels
    grid = np.zeros((4, N, N, N), dtype=np.float32)
    rng = np.random.default_rng(1)
    for ch, pts, tris in meshes:
        a, b, c = pts[tris[:, 0]], pts[tris[:, 1]], pts[tris[:, 2]]
        area = 0.5 * np.linalg.norm(np.cross(b - a, c - a), axis=1) * scale * scale
        count = np.clip(np.ceil(area * 3.0), 1, 64).astype(np.int64)  # ~3 samples per voxel^2
        rep = np.repeat(np.arange(len(tris)), count)
        u, v = rng.random(len(rep)), rng.random(len(rep))
        flip = u + v > 1.0
        u[flip], v[flip] = 1.0 - u[flip], 1.0 - v[flip]
        p = a[rep] + (b[rep] - a[rep]) * u[:, None] + (c[rep] - a[rep]) * v[:, None]
        q = np.floor(p * scale + N / 2).astype(np.int64)
        ok = np.all((q >= 0) & (q < N), axis=1)
        q = q[ok]
        w = (area[rep] / count[rep])[ok]                # each triangle contributes its area
        np.add.at(grid[ch], (q[:, 2], q[:, 1], q[:, 0]), w)
    for ch in range(4):
        g = grid[ch]
        for _ in range(3):
            for ax in range(3):
                g = box_blur(g, 1, ax)
        hi = np.percentile(g[g > 0], 99.7) if np.any(g > 0) else 1.0
        grid[ch] = np.sqrt(np.clip(g / hi, 0.0, 1.0))
    rgba = np.round(grid * 255.0).astype(np.uint8)          # (4, z, y, x)
    rgba = np.transpose(rgba, (1, 2, 3, 0)).copy()          # z, y, x, channel -> x fastest
    os.makedirs(out_dir, exist_ok=True)
    rgba.tofile(os.path.join(out_dir, "casa_volume.raw"))
    meta = {"size": N, "format": "RGBA8, x fastest",
            "channels": ["silicon (Chandra)", "iron (Chandra Fe-K)", "argon/neon infrared (Spitzer)",
                         "fast optical knots + Si jets"],
            "radius_voxels": N * 0.47, "data_units_radius": float(rmax),
            "centre": "central compact object (neutron star)",
            "source": "NASA/CXC/SAO; Delaney et al. 2010, ApJ 725, 2038"}
    with io.open(os.path.join(out_dir, "casa_volume.json"), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)
    print(f"casa_volume: {N}^3, rmax {rmax:.1f} data units, {rgba.nbytes / 2**20:.1f} MiB")


if __name__ == "__main__":
    main()
