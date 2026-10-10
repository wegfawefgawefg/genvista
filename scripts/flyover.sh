#!/bin/sh
# A straight glide over the lake: frames from genvista, a 1080p video with the
# pixels blown up nearest-neighbour, and the same with its synthesised sound.
#   FRAMES=360 TO=72 scripts/flyover.sh [more genvista args, e.g. --seed 7]
set -e
cd "$(dirname "$0")/.."
frames=${FRAMES:-360}
to=${TO:-72}
dur=$(python3 -c "print($frames / 30)")
rm -rf out/fly
./build/genvista --w 640 --h 360 --scale 1 --fly "$frames" --fly-to "$to" --frames-dir out/fly "$@"
ffmpeg -loglevel error -y -framerate 30 -i out/fly/frame_%04d.png \
    -vf "scale=1920:1080:flags=neighbor" -c:v libx264 -crf 14 -preset slow \
    -pix_fmt yuv420p -movflags +faststart out/flyover.mp4
python3 scripts/ambience.py --dur "$dur" --to "$to" --out out/ambience.wav
ffmpeg -loglevel error -y -i out/flyover.mp4 -i out/ambience.wav -map 0:v -map 1:a \
    -c:v copy -c:a aac -b:a 256k -shortest -movflags +faststart out/flyover-sound.mp4
echo "out/flyover.mp4 out/flyover-sound.mp4"
