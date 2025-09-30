#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.." || exit 1
REPO_ROOT="$(pwd)"
ARCHIVE_DIR="$REPO_ROOT/archive"

echo "Repository cleanup script"
echo "Repository root: $REPO_ROOT"
echo

# Preview
echo "Files that will be REMOVED:"
find . -maxdepth 2 -type f \( -name "SoftwareRenderer.exe" -o -name "*.o" -o -name "*.exe" -o -name "frame_*.ppm" \) -print || true
find . -type f \( -name "*~" -o -name "*.swp" -o -name "Thumbs.db" -o -name ".DS_Store" \) -print || true
test -f MakeFile && echo "MakeFile (capital F) will be removed if present."
echo
if [ -d "sdl_import" ]; then
  echo "sdl_import directory found."
  echo "It contains:"
  ls -la sdl_import || true
  echo
  echo "Default action: MOVE sdl_import -> $ARCHIVE_DIR/sdl_import (safe)."
fi

read -p "Proceed with cleanup? (type 'yes' to proceed): " confirm
if [ "$confirm" != "yes" ]; then
  echo "Aborted by user."
  exit 0
fi

# Create archive dir
mkdir -p "$ARCHIVE_DIR"

# 1) Remove build artifacts
echo "Removing binaries and object files..."
find . -maxdepth 2 -type f \( -name "SoftwareRenderer.exe" -o -name "*.o" -o -name "*.exe" -o -name "frame_*.ppm" \) -print -exec rm -f {} \;

# 2) Editor/OS junk
echo "Removing editor/OS junk files..."
find . -type f \( -name "*~" -o -name "*.swp" -o -name "Thumbs.db" -o -name ".DS_Store" \) -print -exec rm -f {} \;

# 3) Remove wrong-cap MakeFile
if [ -f MakeFile ]; then
  echo "Removing MakeFile (capital F)"
  rm -f MakeFile
fi

# 4) Move sdl_import to archive (if present)
if [ -d "sdl_import" ]; then
  echo "Moving sdl_import to $ARCHIVE_DIR/sdl_import"
  mkdir -p "$ARCHIVE_DIR"
  mv sdl_import "$ARCHIVE_DIR/sdl_import"
fi

# 5) Optional: cleanup temporary .vscode caches (but keep launch/tasks)
if [ -d ".vscode" ]; then
  echo "Removing .vscode temporary files (keep launch.json/tasks.json)..."
  find .vscode -maxdepth 1 -type f \( -name "*.log" -o -name ".browse.*" \) -print -exec rm -f {} \; || true
fi

# 6) Final status
echo "Cleanup complete."
echo "Archive directory contents:"
ls -la "$ARCHIVE_DIR" || true
echo
echo "Run 'git status' to review removals, or 'git checkout -- <file>' to restore files from git if needed."
