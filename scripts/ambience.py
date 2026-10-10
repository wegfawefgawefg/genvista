#!/usr/bin/env python3
"""The flyover's sound: a mountain breeze, spruces soughing on either bank, the lake
lapping under you and against the far shore as you near it, snowmelt far off,
chickadees, a raven echoing off the slopes, a fly zipping past and a trout rising.

Everything is synthesised (the first parts began as Mans In My House's tools/audio) and placed
along the same glide as src/flyover.cpp, so the banks pass by in stereo and the shore
comes up as the picture does.

    python3 scripts/ambience.py [--dur 12] [--to 72] [--seed 1] [--out out/ambience.wav]
"""
import argparse
import wave

import numpy as np
from scipy import signal

SR = 48000

# ---------------------------------------------------------------- parts


def n_of(dur):
    return int(round(dur * SR))


def tt(dur):
    return np.arange(n_of(dur)) / SR


def buf(dur):
    return np.zeros(n_of(dur))


def norm(x):
    return x / (np.max(np.abs(x)) + 1e-12)


def place(dst, src, at, gain=1.0):
    i = n_of(at)
    if i >= len(dst) or i < 0:
        return
    n = min(len(src), len(dst) - i)
    dst[i:i + n] += gain * src[:n]


def noise(rng, dur):
    return rng.uniform(-1, 1, n_of(dur))


def lp(x, fc, order=2):
    return signal.sosfilt(signal.butter(order, fc, 'lowpass', fs=SR, output='sos'), x)


def hp(x, fc, order=2):
    return signal.sosfilt(signal.butter(order, fc, 'highpass', fs=SR, output='sos'), x)


def bp(x, lo, hi, order=2):
    return signal.sosfilt(signal.butter(order, [lo, hi], 'bandpass', fs=SR, output='sos'), x)


def smooth_noise(rng, dur, fc):
    return norm(lp(rng.standard_normal(n_of(dur)), fc))


def swell(rng, dur, fc, lo):
    """Slow random gain between lo and 1."""
    return lo + (1 - lo) * (0.5 + 0.5 * smooth_noise(rng, dur, fc))


def env(dur, attack, tau):
    t = tt(dur)
    a = np.clip(t / attack, 0, 1) if attack > 0 else 1.0
    return a * np.exp(-np.maximum(t - attack, 0) / tau)


def reso(x, f, bw):
    """Fixed two-pole resonator."""
    r = np.exp(-np.pi * bw / SR)
    b1, b2 = 2 * r * np.cos(2 * np.pi * f / SR), -r * r
    return signal.lfilter([1 - b1 - b2], [1, -b1, -b2], x)


def sweep_reso(x, f0, f1, bw, pieces=24):
    """A resonator gliding from f0 to f1, in short fixed pieces (fast enough for laps)."""
    out = np.zeros(len(x))
    edges = np.linspace(0, len(x), pieces + 1).astype(int)
    zi = np.zeros(2)
    for k in range(pieces):
        f = f0 + (f1 - f0) * k / max(1, pieces - 1)
        r = np.exp(-np.pi * bw / SR)
        b1, b2 = 2 * r * np.cos(2 * np.pi * f / SR), -r * r
        seg, zi = signal.lfilter([1 - b1 - b2], [1, -b1, -b2], x[edges[k]:edges[k + 1]], zi=zi)
        out[edges[k]:edges[k + 1]] = seg
    return out


def sine(freq):
    return np.sin(2 * np.pi * np.cumsum(freq) / SR)


def bubble(f0, tau):
    """A water plip: a sine whose pitch rises as the bubble closes."""
    d = tau * 6
    t = tt(d)
    return sine(f0 * (1 + 0.5 * (1 - np.exp(-t / (tau * 1.5))))) * env(d, 0.0005, tau)


def space(rng, x, tau, fc, wet):
    """Dry plus a decaying low-passed noise tail, the same length as the input."""
    d = tau * 5
    ir = lp(noise(rng, d), fc) * np.exp(-tt(d) / tau)
    ir[:n_of(0.004)] = 0
    w = signal.fftconvolve(x, ir)[:len(x)]
    return x + wet * w * (np.max(np.abs(x)) / (np.max(np.abs(w)) + 1e-12))


def pan(x, p):
    """Equal-power pan; p is -1 (left) .. 1 (right), a number or a curve."""
    a = (np.asarray(p) + 1) * np.pi / 4
    return np.stack([x * np.cos(a), x * np.sin(a)])


# ---------------------------------------------------------------- the glide


def glide(t):
    """The same easing as src/flyover.cpp."""
    smooth = t * t * (3 - 2 * t)
    return t + (smooth - t) * 0.6


def path(dur, to_z):
    t = tt(dur) / dur
    return to_z * glide(t)


def nearness(z, x, z_at, ref):
    """1 at ref metres or closer, falling off as 1/distance."""
    d = np.hypot(x, z_at - z)
    return np.minimum(1.0, ref / d)


# Where the scene's things are, roughly (metres; x right of the eye, z ahead).
LEFT_STAND = (-48.0, 102.0)
RIGHT_STAND = (34.0, 105.0)
RIGHT_BIG = (50.0, 72.0)
FAR_SHORE_Z = 79.0

# ---------------------------------------------------------------- layers


def breeze(rng, dur, gust):
    """The mountain air: a soft low wash in each ear, swelling with the gusts."""
    out = []
    for _ in range(2):
        wash = norm(lp(noise(rng, dur), 380)) * gust
        body = norm(reso(noise(rng, dur), 260, 120)) * gust ** 1.5
        out.append(0.8 * wash + 0.25 * body)
    return np.stack(out)


def rustle(rng, dur, density):
    """Dry little grains of needles and twigs knocking, thicker where density is high."""
    out = buf(dur)
    peak = float(np.max(density))
    at = rng.exponential(1 / 60)
    while at < dur:
        if rng.uniform() < density[min(n_of(at), len(density) - 1)] / peak:
            d = rng.uniform(0.003, 0.012)
            grain = bp(noise(rng, d), rng.uniform(1800, 3200), rng.uniform(4500, 7500)) * env(d, 0.0005, d * 0.3)
            place(out, grain, at, rng.uniform(0.3, 1.0))
        at += rng.exponential(1 / 60)
    return norm(out)


def needles(rng, dur, gust, z):
    """Spruces in the breeze: a soft sigh through the needles (the soughing) with dry
    rustle grains in the gusts, from each bank as it comes alongside."""
    out = np.zeros((2, n_of(dur)))
    for (x, zs), p in ((LEFT_STAND, -0.75), (RIGHT_STAND, 0.6), (RIGHT_BIG, 0.9)):
        sway = gust * (0.75 + 0.25 * (0.5 + 0.5 * smooth_noise(rng, dur, 1.5)))
        sigh = norm(reso(bp(noise(rng, dur), 500, 4200), 1400, 900)) * sway ** 2
        grains = rustle(rng, dur, sway ** 3)
        near = nearness(z, x, zs, 40.0)
        out += pan((sigh + 0.35 * grains * sway ** 2) * near, p)
    return out


def lap(rng, big):
    """One wavelet against something: a resonant slosh with a plip or two in it."""
    d = rng.uniform(0.3, 0.6) * (1 + 0.4 * big)
    f0 = rng.uniform(220, 320) * (1 - 0.15 * big)
    y = norm(sweep_reso(noise(rng, d), f0, f0 * rng.uniform(1.8, 2.4), 160)) * env(d, rng.uniform(0.03, 0.07),
                                                                                  d * 0.28)
    for _ in range(rng.integers(0, 3)):
        place(y, bubble(rng.uniform(700, 1500), rng.uniform(0.008, 0.02)), rng.uniform(0.02, d * 0.5),
              rng.uniform(0.08, 0.25))
    return y


def lake(rng, dur, z):
    """Under you: a quiet rocking bed and small laps all round. Ahead: the shore, louder
    and more often as it comes up."""
    out = np.zeros((2, n_of(dur)))
    rock = swell(rng, dur, 0.6, 0.3)
    for k in range(2):
        out[k] += 0.35 * norm(lp(noise(rng, dur), 420)) * rock
    # small laps all round, a steady patter
    at = rng.uniform(0.0, 0.3)
    while at < dur:
        place_st(out, lap(rng, 0.0), at, rng.uniform(0.15, 0.35), rng.uniform(-0.8, 0.8))
        at += rng.uniform(0.25, 0.7)
    # the far shore: busier and nearer as the glide ends
    shore = np.interp(np.linspace(0, dur, 200), tt(dur), np.minimum(1.0, 12.0 / np.maximum(FAR_SHORE_Z - z, 1)))
    at = 0.2
    while at < dur:
        s = float(np.interp(at, np.linspace(0, dur, 200), shore))
        place_st(out, lap(rng, 1.0), at, 0.25 + 0.9 * s, rng.uniform(-0.35, 0.35))
        at += rng.uniform(0.5, 1.1) * (1.4 - s)
    # the odd plip close by
    at = rng.exponential(1.2)
    while at < dur:
        place_st(out, bubble(rng.uniform(900, 2200), rng.uniform(0.006, 0.016)), at, rng.uniform(0.1, 0.3),
                 rng.uniform(-0.9, 0.9))
        at += rng.exponential(1.2)
    return out


def place_st(dst, mono, at, gain, p):
    st = pan(mono, p)
    for k in range(2):
        place(dst[k], st[k], at, gain)


def chickadee(rng):
    """A mountain chickadee: "fee-bee-bee", pure whistles stepping down, a little vibrato."""
    top = rng.uniform(3600, 4000)
    notes = [(top, rng.uniform(0.3, 0.38)), (top * 0.86, 0.17), (top * 0.86, 0.17)]
    out = buf(1.2)
    at = 0.0
    for f, d in notes:
        t = tt(d)
        vib = 1 + 0.004 * np.sin(2 * np.pi * 7 * t)
        bend = 1 - 0.015 * t / d
        tone = sine(f * vib * bend) * np.sin(np.pi * t / d) ** 1.5
        place(out, tone, at)
        at += d + 0.05
    return out


def birds(rng, dur):
    """Three or four far calls from the trees, each with a faint echo off the slopes."""
    out = np.zeros((2, n_of(dur)))
    times = np.sort(rng.uniform(0.8, dur - 1.5, 4))
    for at in times:
        call = space(rng, chickadee(rng), 0.25, 5000, 0.35)
        place_st(out, call, at, rng.uniform(0.05, 0.09), rng.choice([-0.8, -0.6, 0.55, 0.85]))
    return out


def saw(freq):
    return 2 * ((np.cumsum(freq) / SR) % 1) - 1


def cascade(rng, dur):
    """Snowmelt coming down off the slopes far away: a steady soft roar, wide and low."""
    out = []
    for _ in range(2):
        roar = norm(bp(rng.standard_normal(n_of(dur)), 220, 2400, 1))
        out.append(lp(roar, 1800) * swell(rng, dur, 0.4, 0.8))
    return np.stack(out) * np.array([[0.8], [1.0]])


def airflow(rng, dur, z):
    """The glide itself: a faint rush that follows the speed."""
    speed = np.gradient(z) * SR
    speed = speed / (np.max(speed) + 1e-12)
    out = []
    for _ in range(2):
        out.append(norm(bp(noise(rng, dur), 120, 900)) * speed ** 1.5)
    return np.stack(out)


def raven(rng):
    """One raven's "kraa": a rough nasal tone through three formants, falling at the end."""
    d = rng.uniform(0.32, 0.4)
    t = tt(d)
    f0 = np.interp(t, [0, d * 0.3, d], [330, 360, 250]) * (1 + 0.03 * smooth_noise(rng, d, 60))
    src = saw(f0) * (0.75 + 0.25 * np.sin(2 * np.pi * 38 * t)) + 0.3 * noise(rng, d)
    y = sum(g * reso(src, f, bw) for f, bw, g in ((950, 180, 1.0), (1500, 220, 0.7), (2600, 400, 0.35)))
    return norm(y) * np.sin(np.pi * t / d) ** 0.6


def ravens(rng, dur):
    """A raven over the far slopes, twice, each call coming back off the mountains."""
    out = np.zeros((2, n_of(dur)))
    at = rng.uniform(2.0, 2.8)
    for k in range(2):
        call = raven(rng)
        place_st(out, call, at + k * 0.55, 0.11, -0.3)
        echo = lp(call, 2200)
        place_st(out, echo, at + k * 0.55 + 0.62, 0.035, 0.35)
        place_st(out, lp(echo, 1400), at + k * 0.55 + 1.5, 0.015, -0.1)
    return out


def fly_by(rng, at, dur):
    """A fly zipping past the lens left to right: buzzing wings, Doppler, close then gone."""
    d = 1.7
    t = tt(d)
    x = np.interp(t, [0, d], [-3.5, 3.0]) + 0.3 * np.sin(2 * np.pi * 1.3 * t)
    y, zf = 0.25 + 0.15 * np.sin(2 * np.pi * 0.9 * t), 1.1
    r = np.sqrt(x ** 2 + y ** 2 + zf ** 2)
    shift = 343 / (343 + np.gradient(r) * SR)
    f0 = 190 * (1 + 0.04 * smooth_noise(rng, d, 6)) * shift
    wings = saw(f0) + 0.5 * saw(f0 * 2.01)
    buzz = bp(wings, 160, 5000) * (0.7 + 0.3 * np.sin(2 * np.pi * f0 * t * 0.5))
    buzz = norm(buzz) * np.minimum(1.0, 0.6 / r) ** 1.4 * np.sin(np.pi * t / d) ** 0.3
    out = np.zeros((2, n_of(dur)))
    st = pan(buzz, np.clip(x / r, -1, 1))
    for k in range(2):
        place(out[k], st[k], at, 1.0)
    return out


def fish_jump(rng, dur, at, p):
    """A trout rising off to the side: a hollow plop, a slap of spray, droplets falling back."""
    out = np.zeros((2, n_of(dur)))
    d = 0.5
    t = tt(d)
    plop = sine(70 + 140 * np.exp(-t / 0.012)) * env(d, 0.001, 0.05)
    spray = lp(noise(rng, d), 3500) * env(d, 0.002, 0.03)
    hit = norm(plop + 0.5 * spray)
    place_st(out, hit, at, 0.4, p)
    for _ in range(9):
        place_st(out, bubble(rng.uniform(800, 2400), rng.uniform(0.005, 0.014)), at + rng.uniform(0.12, 0.6),
                 rng.uniform(0.05, 0.18), p + rng.uniform(-0.1, 0.1))
    # the rings spreading: a few soft laps after
    for k in range(3):
        place_st(out, lap(rng, 0.0), at + 0.5 + 0.35 * k, 0.12, p)
    return out


# ---------------------------------------------------------------- mix


def mix(dur, to_z, seed):
    rng = np.random.default_rng(seed)
    z = path(dur, to_z)
    gust = swell(rng, dur, 0.22, 0.25)
    layers = {
        'breeze': (breeze(rng, dur, gust), 0.26),
        'needles': (needles(rng, dur, gust, z), 0.3),
        'lake': (lake(rng, dur, z), 0.55),
        'birds': (birds(rng, dur), 1.0),
        'cascade': (cascade(rng, dur), 0.05),
        'airflow': (airflow(rng, dur, z), 0.06),
        'ravens': (ravens(rng, dur), 2.0),
        'fly': (fly_by(rng, dur * 0.36, dur), 0.45),
        'fish': (fish_jump(rng, dur, dur * 0.58, 0.55), 1.0),
    }
    out = np.zeros((2, n_of(dur)))
    for name, (x, gain) in layers.items():
        out += gain * x
    # fade in and out
    fade_in, fade_out = n_of(0.8), n_of(1.2)
    out[:, :fade_in] *= np.linspace(0, 1, fade_in) ** 2
    out[:, -fade_out:] *= np.linspace(1, 0, fade_out) ** 2
    # set the loudness, then let a soft limiter round off the few peaks (the fish)
    out *= 10 ** (-20 / 20) / (np.sqrt(np.mean(out ** 2)) + 1e-12)
    knee = 10 ** (-6 / 20)
    out = np.where(np.abs(out) > knee, np.sign(out) * (knee + (1 - knee) * np.tanh((np.abs(out) - knee) / (1 - knee))),
                   out)
    out *= 10 ** (-1 / 20) / max(1.0, float(np.max(np.abs(out))))
    for name, (x, gain) in layers.items():
        level = 20 * np.log10(np.sqrt(np.mean((gain * x) ** 2)) + 1e-12)
        print(f'  {name:8s} rms {level:6.1f} dB (before the master)')
    rms = 20 * np.log10(np.sqrt(np.mean(out ** 2)))
    peak = 20 * np.log10(np.max(np.abs(out)))
    print(f'  master   rms {rms:6.1f} dBFS, peak {peak:5.1f} dBFS')
    return out


def write_wav(path_out, st):
    data = (np.clip(st.T, -1, 1) * 32767).astype('<i2')
    with wave.open(path_out, 'wb') as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(data.tobytes())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dur', type=float, default=12.0)
    ap.add_argument('--to', type=float, default=72.0)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--out', default='out/ambience.wav')
    args = ap.parse_args()
    write_wav(args.out, mix(args.dur, args.to, args.seed))
    print(args.out)


if __name__ == '__main__':
    main()
