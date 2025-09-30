#!/usr/bin/env bash
set -euo pipefail

# prepare_release.sh
# Usage: ./scripts/prepare_release.sh
#
# Builds the project (make) and packages a release zip for Windows (x64).
# It tries to include the SDL2 runtime DLL from /mingw64/bin (if present).
# If 'zip' is not installed it falls back to Python zipping.

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

BUILD_CMD="make"
TARGET_EXE="SoftwareRenderer.exe"
RELEASE_DIR="$REPO_ROOT/release"
TMPDIR="$(mktemp -d "${TMPDIR:-/tmp}/swrender_release.XXXX")"
TIMESTAMP="$(date +%Y%m%d-%H%M)"
ARCHIVE_NAME="SoftwareRenderer_WIN_x64_${TIMESTAMP}.zip"
ARCHIVE_PATH="$RELEASE_DIR/$ARCHIVE_NAME"

echo "Repo root: $REPO_ROOT"
echo "Temporary packaging dir: $TMPDIR"
echo

# 0) Pre-checks
if ! command -v g++ >/dev/null 2>&1; then
  echo "ERROR: g++ not found in PATH. Install MinGW-w64 toolchain or run in MSYS2 MinGW64 shell."
  exit 1
fi

if ! command -v make >/dev/null 2>&1; then
  echo "ERROR: make not found in PATH. Install make (pacman -S make) or run in MSYS2 MinGW64 shell."
  exit 1
fi

echo "Cleaning previous build artifacts..."
$BUILD_CMD clean || true

echo "Building release executable..."
$BUILD_CMD

# 1) Prepare release folder
mkdir -p "$TMPDIR/release_files"
cd "$TMPDIR/release_files"

# Copy the executable
if [ ! -f "$REPO_ROOT/$TARGET_EXE" ]; then
  echo "ERROR: expected built exe not found at $REPO_ROOT/$TARGET_EXE"
  exit 1
fi
cp -v "$REPO_ROOT/$TARGET_EXE" .

# 2) Try to find SDL2 runtime DLL (common MSYS2 location)
SDL2_DLL_CANDIDATES=(
  "/mingw64/bin/SDL2.dll"
  "/usr/mingw64/bin/SDL2.dll"
  "$REPO_ROOT/sdl_import/SDL2.dll"
  "$REPO_ROOT/sdl_import/SDL3.dll"
)

FOUND_DLL=""
for cand in "${SDL2_DLL_CANDIDATES[@]}"; do
  if [ -f "$cand" ]; then
    FOUND_DLL="$cand"
    break
  fi
done

if [ -n "$FOUND_DLL" ]; then
  echo "Including SDL runtime DLL: $FOUND_DLL"
  cp -v "$FOUND_DLL" .
else
  echo "Warning: SDL2 runtime DLL not found in standard locations."
  echo "If the packaged executable needs SDL2.dll, add it to the release or instruct users to install SDL2."
fi

# 3) Copy README, LICENSE, screenshots if present
for f in README.md README.txt LICENSE LICENSE.md frame_000.png frame_000.ppm; do
  if [ -f "$REPO_ROOT/$f" ]; then
    cp -v "$REPO_ROOT/$f" .
  fi
done

# 4) Add a small release metadata file
cat > release-info.txt <<EOF
SoftwareRenderer - Release
Date: $(date -u +"%Y-%m-%d %H:%M:%SZ")
Host: $(hostname) / $(uname -a)
Included:
- $TARGET_EXE
- SDL2 runtime: $( [ -n "$FOUND_DLL" ] && echo "$(basename "$FOUND_DLL")" || echo "NOT_INCLUDED" )
EOF

echo "Created release-info.txt"

# 5) Create release folder and zip
mkdir -p "$RELEASE_DIR"

echo "Creating zip archive $ARCHIVE_PATH ..."

# Prefer zip if installed
if command -v zip >/dev/null 2>&1; then
  (cd "$TMPDIR/release_files" && zip -r -q "$ARCHIVE_PATH" .)
else
  # Fallback to Python zipping (should be present)
  if command -v python >/dev/null 2>&1; then
    python - <<PY
import os, zipfile, sys
src = "$TMPDIR/release_files"
dst = "$ARCHIVE_PATH"
with zipfile.ZipFile(dst, 'w', compression=zipfile.ZIP_DEFLATED) as z:
    for root, dirs, files in os.walk(src):
        for f in files:
            full = os.path.join(root, f)
            arcname = os.path.relpath(full, src)
            z.write(full, arcname)
print("Wrote", dst)
PY
  else
    echo "ERROR: neither 'zip' nor 'python' found to create archive."
    exit 1
  fi
fi

# 6) Generate checksum
if command -v sha256sum >/dev/null 2>&1; then
  (cd "$RELEASE_DIR" && sha256sum "$(basename "$ARCHIVE_PATH")" > "${ARCHIVE_PATH}.sha256")
elif command -v shasum >/dev/null 2>&1; then
  (cd "$RELEASE_DIR" && shasum -a 256 "$(basename "$ARCHIVE_PATH")" > "${ARCHIVE_PATH}.sha256")
else
  echo "Note: no sha256 tool found; skipping checksum creation."
fi

# 7) Cleanup temp dir
rm -rf "$TMPDIR"

echo
echo "Release package created: $ARCHIVE_PATH"
if [ -f "${ARCHIVE_PATH}.sha256" ]; then
  echo "Checksum: ${ARCHIVE_PATH}.sha256"
fi

echo "Done."
