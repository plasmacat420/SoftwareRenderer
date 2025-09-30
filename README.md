Software Renderer (C++17 + SDL2)

A tiny software renderer written in C++17, using SDL2 for windowing and input.
It draws a rotating 3D cube entirely on the CPU, pixel by pixel — no GPU acceleration.


(rotating cube, rendered in software)

✨ Features

Custom framebuffer + line rasterizer

3D → 2D perspective projection

Real-time rotation (~60 FPS)

Export frames → GIF/MP4 with --record

🔧 Build & Run (MSYS2 MinGW64)

Install dependencies:

pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make ffmpeg


Clone and build:

git clone https://github.com/plasmacat420/SoftwareRenderer.git
cd SoftwareRenderer
make


Run:

./SoftwareRenderer.exe

🎥 Export a GIF

Record 180 frames at 30 FPS (≈6 sec) and build a GIF:

./SoftwareRenderer.exe --width 800 --height 600 --record 180 --record-fps 30
./scripts/make_gif.sh frames 30 cube.gif cube.mp4

📦 Releases

Prebuilt Windows binaries are available under Releases
.

🔮 Next Steps

Filled triangle rasterization

Depth buffering (z-buffer)

Flat shading & textures

Maybe even a software ray tracer 🚀