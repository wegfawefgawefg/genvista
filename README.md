# genvista

![a mountain lake mirroring spruces and cumulus, ray cast out of crude convex solids](docs/vista.png)

A mountain lake from a photo I had as my desktop wallpaper, rebuilt as a tiny
ray caster in the style of the sprite renderer from my game Mans In My House.
Nothing here is the photo: the image above is what the code makes.

How it draws:

- every object is a convex solid, an intersection of planes (lumps, spruce
  skirts, boxes, peaks), hit with a slab test, one ray per pixel
- flat facets lit in four bands, hard shadows, blue-leaning shade, haze
- 3D grain in each solid's own space, per-facet brightness jitter
- hatching in the darkest band, ink lines where one solid passes in front of
  another, outlines against the sky
- a still lake at y = 0 that mirrors the lot, darks darker, whites kept
- a wobbling ray grid, then a 4x4 Bayer dither onto a palette taken from the
  photo, blown up nearest-neighbour

The layout comes from the photo by hand: things are placed by image position
and distance (`place(camera, u, v, z)`); the lumps, puffs and skirts grow from
the seed.

    cmake -S . -B build -G Ninja && ninja -C build
    ./build/genvista                                   # 640x480 x5 -> out/vista.png (3200x2400)
    ./build/genvista --w 640 --h 360 --scale 3 --out out/vista-1920x1080.png
    ./build/genvista --seed 7 --no-palette --dither 0  # other lumps, raw colour

A flyover, a straight level glide just above the water toward the far shore
(12 s at 30 fps, 1080p, under a minute on 32 threads), with a synthesised
soundscape that follows the glide: the breeze, spruces soughing on each bank as
it comes alongside, the lake lapping under you and against the far shore as it
nears, snowmelt far off, chickadees, a raven echoing off the slopes, a fly
zipping past the lens and a trout rising. Needs ffmpeg, and Python with numpy
and scipy for the sound (`scripts/ambience.py`).

    scripts/flyover.sh                    # -> out/flyover.mp4, out/flyover-sound.mp4
    FRAMES=600 TO=75 scripts/flyover.sh --fly-height 1.5

A wider image keeps the full width of the 4:3 layout and shows a band of it
starting `--top` (0.06) of the way down.
