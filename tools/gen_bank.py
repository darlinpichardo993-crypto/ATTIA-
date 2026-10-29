#!/usr/bin/env python3
"""ATTIA Resynth Wave Bank generator.
11 familias x 1000 ondas (20 series de 50 ondas que se transforman suavemente) = 11.000 ondas
de un ciclo (2048 muestras, float32). Salida: attia_bank.bin (~90 MB).  Uso: gen_bank.py salida.bin"""
import sys, numpy as np

N = 2048
FAM = 11
SERIES = 20
STEPS = 50
H = 48  # armonicos maximos guardados

def norm(w):
    w = w - w.mean(axis=-1, keepdims=True)
    pk = np.abs(w).max(axis=-1, keepdims=True)
    return (w / np.maximum(pk, 1e-9) * 0.9).astype(np.float32)

def from_spec(mag, ph):
    spec = np.zeros((mag.shape[0], N // 2 + 1), dtype=np.complex128)
    spec[:, 1:H + 1] = mag * np.exp(1j * ph)
    return np.fft.irfft(spec, n=N, axis=-1) * N / 2

def bandlimit(w, hmax=H):
    s = np.fft.rfft(w, axis=-1)
    s[:, hmax + 1:] = 0
    return np.fft.irfft(s, n=N, axis=-1)

h = np.arange(1, H + 1, dtype=np.float64)

def anchors(fam, rng):
    ph = np.zeros(H)
    if fam == 0:   # Sine Bloom
        m = rng.random(H) ** 3 * h ** -1.5; m[0] = 1.0
    elif fam == 1: # Odd Glass
        sl = rng.uniform(0.8, 2.2); m = h ** -sl; m[1::2] *= rng.uniform(0.0, 0.12)
    elif fam == 2: # Formant Choir
        c = [rng.uniform(2, 6), rng.uniform(6, 14), rng.uniform(14, 26)]
        wd = [rng.uniform(1.2, 3), rng.uniform(1.5, 3.5), rng.uniform(2, 5)]
        m = sum(np.exp(-((h - ci) / wi) ** 2) * a for ci, wi, a in zip(c, wd, [1, 0.7, 0.4])) * h ** -0.3
        m[0] = max(m[0], 0.5)
    elif fam == 3: # Bowed Strings
        sl = rng.uniform(0.55, 1.1)
        m = h ** -sl * (1 + 0.35 * np.cos(h * rng.uniform(0.2, 1.1) + rng.uniform(0, 6)))
        for _ in range(2):
            m = m + np.exp(-((h - rng.uniform(3, 16)) / rng.uniform(1, 2.5)) ** 2) * 0.35
        ph = rng.uniform(-0.4, 0.4, H)
    elif fam == 4: # Plucked Strings
        pos = rng.uniform(0.08, 0.5); s = rng.uniform(1.0, 1.7)
        m = np.abs(np.sin(np.pi * h * pos)) / h ** s + 0.01 * h ** -1.0
    elif fam == 5: # Sub Bass
        m = h ** -rng.uniform(2.5, 4.0); m[0] = 1.0; m[1] = rng.uniform(0.05, 0.6); m[2] = rng.uniform(0.0, 0.35)
    elif fam == 7: # Organ Drawbars
        m = np.zeros(H)
        for k, a in zip([1, 2, 3, 4, 5, 6, 8, 10, 12, 16], rng.random(10)):
            m[k - 1] = a
        m[0] = max(m[0], 0.4)
    elif fam == 9: # Warm Tines
        m = np.zeros(H); m[0] = 1.0; m[1] = rng.uniform(0.15, 0.75); m[2] = rng.uniform(0.05, 0.5)
        b = rng.integers(5, 10)
        m = m + np.exp(-((h - b) / rng.uniform(0.6, 1.5)) ** 2) * rng.uniform(0.1, 0.5)
        m[3:] += h[3:] ** -2.2 * 0.2
    else:          # 10 Spectral Rooms
        m = rng.random(H) ** rng.uniform(0.6, 2.0) * h ** -rng.uniform(0.2, 0.9); ph = rng.uniform(0, 2 * np.pi, H)
    return m, ph

def td_series(fam, rng):
    p = np.arange(N) / N
    t = np.linspace(0, 1, STEPS)[:, None]
    if fam == 6:  # FM Metal
        r1 = rng.choice([1, 2, 3, 4, 5, 7]); r2 = rng.choice([1, 2, 3, 6, 8])
        i0, i1 = rng.uniform(0.2, 2), rng.uniform(1, 7); j0, j1 = rng.uniform(0, 1), rng.uniform(0, 3)
        I = i0 + (i1 - i0) * t; J = j0 + (j1 - j0) * t
        w = np.sin(2 * np.pi * p + I * np.sin(2 * np.pi * r1 * p) + J * np.sin(2 * np.pi * r2 * p))
    else:         # Alien Birds
        k1, k2 = int(rng.integers(2, 9)), int(rng.integers(1, 6))
        a0, a1 = rng.uniform(0.5, 3), rng.uniform(2, 9); c0, c1 = rng.uniform(0, 2), rng.uniform(1, 6)
        A = a0 + (a1 - a0) * t; C = c0 + (c1 - c0) * t
        w = np.sin(2 * np.pi * p + A * np.sin(2 * np.pi * k1 * p + C * np.sin(2 * np.pi * k2 * p)))
    return bandlimit(w, 40)

def main(out):
    rng = np.random.default_rng(20261029)
    waves = np.empty((FAM * SERIES * STEPS, N), dtype=np.float32)
    k = 0
    for fam in range(FAM):
        for s in range(SERIES):
            if fam in (6, 8):
                w = td_series(fam, rng)
            else:
                (ma, pa), (mb, pb) = anchors(fam, rng), anchors(fam, rng)
                t = np.linspace(0, 1, STEPS)[:, None]
                w = from_spec(ma * (1 - t) + mb * t, pa * (1 - t) + pb * t)
            waves[k:k + STEPS] = norm(w); k += STEPS
    assert k == 11000
    with open(out, 'wb') as f:
        f.write(b'ATTIABK1'); f.write(np.array([k, N], dtype='<u4').tobytes()); f.write(waves.astype('<f4').tobytes())
    print('banco:', out, k, 'ondas,', round((16 + waves.nbytes) / 1e6, 1), 'MB')

if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'attia_bank.bin')
