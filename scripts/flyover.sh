#!/bin/sh
# A straight glide over the lake: frames from genvista, then a 1080p video
# with the pixels blown up nearest-neighbour. Extra arguments go to genvista
# (--seed 7, --fly-to 60, ...).
set -e
cd "$(dirname "$0")/.."
frames=${FRAMES:-360}
rm -rf out/fly
./build/genvista --w 640 --h 360 --scale 1 --fly "$frames" --frames-dir out/fly "$@"
ffmpeg -loglevel error -y -framerate 30 -i out/fly/frame_%04d.png \
    -vf "scale=1920:1080:flags=neighbor" -c:v libx264 -crf 14 -preset slow \
    -pix_fmt yuv420p -movflags +faststart out/flyover.mp4
echo "out/flyover.mp4"
