#!/usr/bin/env bash
set -euo pipefail

FRAMES_DIR="${1:-frames}"
FPS="${2:-30}"
OUT_GIF="${3:-cube.gif}"
OUT_MP4="${4:-cube.mp4}"

if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg not found. On MSYS2: pacman -S ffmpeg" >&2
  exit 1
fi

# Check frames exist
if ! ls "$FRAMES_DIR"/frame_*.ppm >/dev/null 2>&1; then
  echo "No frames found in '$FRAMES_DIR' (expected frame_####.ppm)" >&2
  exit 1
fi

echo "Creating MP4 at ${OUT_MP4} ..."
ffmpeg -y -framerate "$FPS" -i "$FRAMES_DIR/frame_%04d.ppm" \
  -pix_fmt yuv420p -movflags +faststart "$OUT_MP4"

echo "Creating GIF at ${OUT_GIF} (two-pass palette) ..."
ffmpeg -y -framerate "$FPS" -i "$FRAMES_DIR/frame_%04d.ppm" \
  -vf "palettegen=stats_mode=diff" -f image2 -y /tmp/palette.png

ffmpeg -y -framerate "$FPS" -i "$FRAMES_DIR/frame_%04d.ppm" -i /tmp/palette.png \
  -lavfi "paletteuse=dither=bayer:bayer_scale=5:diff_mode=rectangle" "$OUT_GIF"

echo "Done:"
ls -lh "$OUT_MP4" "$OUT_GIF"

chmod +x scripts/make_gif.sh