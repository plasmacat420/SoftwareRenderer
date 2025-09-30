// src/main.cpp - SDL2-only with recording support (--record N)
#include "Renderer.h"
#include <SDL2/SDL.h>

#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>

// --- Math helpers ---
struct Vec3 { float x,y,z; };
constexpr float PI = 3.14159265358979323846f;

static Vec3 rotateY(const Vec3 &v, float a){
    float s = sinf(a), c = cosf(a);
    return { c*v.x + s*v.z, v.y, -s*v.x + c*v.z };
}
static Vec3 rotateX(const Vec3 &v, float a){
    float s = sinf(a), c = cosf(a);
    return { v.x, c*v.y - s*v.z, s*v.y + c*v.z };
}

// --- Shape generators ---
static void makeCube(std::vector<Vec3> &verts, std::vector<std::pair<int,int>> &edges) {
    verts = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };
    edges = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
}

static void makeTetrahedron(std::vector<Vec3> &verts, std::vector<std::pair<int,int>> &edges) {
    verts = {
        {0,0,1.2f}, {1,0,-0.4f}, {-0.5f,0.87f,-0.4f}, {-0.5f,-0.87f,-0.4f}
    };
    edges = { {0,1},{0,2},{0,3},{1,2},{2,3},{3,1} };
}

static void makeIcosahedron(std::vector<Vec3> &verts, std::vector<std::pair<int,int>> &edges) {
    verts.clear(); edges.clear();
    float phi = (1 + sqrtf(5.0f)) * 0.5f; // golden ratio
    verts = {
        {-1,  phi, 0}, {1,  phi, 0}, {-1, -phi, 0}, {1, -phi, 0},
        {0, -1,  phi}, {0,  1,  phi}, {0, -1, -phi}, {0,  1, -phi},
        { phi, 0, -1}, { phi, 0,  1}, {-phi, 0, -1}, {-phi, 0,  1}
    };
    // Normalize
    for (auto &v: verts) {
        float l = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        v.x/=l; v.y/=l; v.z/=l;
    }
    // Edges (manually defined adjacency)
    edges = {
        {0,1},{0,5},{0,7},{0,10},{0,11},
        {1,5},{1,7},{1,8},{1,9},
        {2,3},{2,4},{2,6},{2,10},{2,11},
        {3,4},{3,6},{3,8},{3,9},
        {4,5},{4,9},{4,11},
        {5,9},{5,11},
        {6,7},{6,8},{6,10},
        {7,8},{7,10},
        {8,9},
        {10,11}
    };
}

static void makeHelix(std::vector<Vec3> &verts, std::vector<std::pair<int,int>> &edges, int N=200) {
    verts.clear(); edges.clear();
    for (int i=0; i<N; i++) {
        float t = i * 0.15f;
        verts.push_back({cosf(t), sinf(t), t*0.05f});
        if (i>0) edges.push_back({i-1,i});
    }
}

static void makeGrid(std::vector<Vec3> &verts, std::vector<std::pair<int,int>> &edges, int N=4) {
    verts.clear(); edges.clear();
    for (int x=0; x<N; x++) {
        for (int y=0; y<N; y++) {
            for (int z=0; z<N; z++) {
                verts.push_back({(float)x-(N-1)/2.0f, (float)y-(N-1)/2.0f, (float)z-(N-1)/2.0f});
            }
        }
    }
    // connect in x, y, z directions
    int idx=0;
    for (int x=0; x<N; x++) {
        for (int y=0; y<N; y++) {
            for (int z=0; z<N; z++, idx++) {
                if (x+1<N) edges.push_back({idx, idx+N*N});
                if (y+1<N) edges.push_back({idx, idx+N});
                if (z+1<N) edges.push_back({idx, idx+1});
            }
        }
    }
}


// --- Simple PPM writer (RGB24) ---
static bool writePPM(const char *path, int W, int H, const unsigned char *rgb) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    size_t bytes = (size_t)W * H * 3;
    bool ok = fwrite(rgb, 1, bytes, f) == bytes;
    fclose(f);
    return ok;
}

int main(int argc, char** argv) {
    // -------- CLI options --------
    int W = 640, H = 480;
    bool saveFrameOnStart = false;
    double runDurationSec = 0.0;            // 0 = run until quit
    int recordCount = 0;                    // 0 = no recording
    int recordFps = 30;                     // target recording FPS
    std::string framesDir = "frames";       // output folder

    for (int i=1; i<argc; ++i) {
        std::string s(argv[i]);
        if (s=="--width" && i+1<argc)  W = std::max(64, atoi(argv[++i]));
        else if (s=="--height" && i+1<argc) H = std::max(64, atoi(argv[++i]));
        else if (s=="--save-frame")    saveFrameOnStart = true;
        else if (s=="--duration" && i+1<argc) runDurationSec = atof(argv[++i]);
        else if (s=="--record" && i+1<argc)   recordCount = std::max(0, atoi(argv[++i]));
        else if (s=="--record-fps" && i+1<argc) recordFps = std::max(1, atoi(argv[++i]));
        else if (s=="--frames-dir" && i+1<argc) framesDir = argv[++i];
        else if (s=="--help" || s=="-h") {
            printf(
              "Usage: %s [--width N] [--height N] [--save-frame]\n"
              "          [--duration seconds] [--record N] [--record-fps X]\n"
              "          [--frames-dir path]\n",
              argv[0]);
            return 0;
        }
    }

    printf("PROGRAM START (W=%d H=%d startSave=%d duration=%.2fs record=%d @ %d FPS, dir='%s')\n",
        W,H, saveFrameOnStart?1:0, runDurationSec, recordCount, recordFps, framesDir.c_str());
    fflush(stdout);

    // Ensure output dir exists if recording
    if (recordCount > 0) {
        std::error_code ec;
        std::filesystem::create_directories(framesDir, ec);
        if (ec) {
            fprintf(stderr, "Failed to create frames dir '%s' (%s)\n",
                    framesDir.c_str(), ec.message().c_str());
            return 1;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: '%s'\n", SDL_GetError()); fflush(stderr);
        return 1;
    }
    printf("SDL_Init OK\n");

    SDL_Window *win = SDL_CreateWindow(
        "Software Renderer (SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow failed: '%s'\n", SDL_GetError());
        SDL_Quit(); return 1;
    }

    SDL_Surface *surface = SDL_GetWindowSurface(win);
    if (!surface) {
        fprintf(stderr, "SDL_GetWindowSurface failed: '%s'\n", SDL_GetError());
        SDL_DestroyWindow(win); SDL_Quit(); return 1;
    }

    Renderer renderer(W, H);

    std::vector<Vec3> verts;
    std::vector<std::pair<int,int>> edges;

    int currentShape = 1; // start with cube
    makeCube(verts, edges);


    const float cameraZ = 4.0f;
    const float fov = 90.0f;
    float f = 1.0f / tanf((fov * 0.5f) * PI / 180.0f);
    float scale = f * (W / 2.0f);

    bool running = true, paused = false, requestedSave = false;
    SDL_Event ev;
    Uint32 lastTicks = SDL_GetTicks();
    float angle = 0.0f;
    int frame = 0;

    // FPS stats
    int framesThisSec = 0; double fps = 0.0;
    auto t0 = std::chrono::steady_clock::now();
    auto runStart = t0;

    // Recording pacing
    const double targetFrameSec = recordCount>0 ? (1.0 / recordFps) : 0.0;
    double accumSec = 0.0; // for fps title update
    bool savedStartFrame = false;
    int recorded = 0;

    while (running) {
        // auto-exit by duration?
        if (runDurationSec > 0.0) {
            double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - runStart).count();
            if (elapsed >= runDurationSec) { printf("Reached duration %.2fs\n", runDurationSec); break; }
        }

        // events
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_1: makeCube(verts, edges); currentShape=1; printf("Shape: Cube\n"); break;
                case SDLK_2: makeTetrahedron(verts, edges); currentShape=2; printf("Shape: Tetrahedron\n"); break;
                case SDLK_3: makeIcosahedron(verts, edges); currentShape=3; printf("Shape: Icosahedron\n"); break;
                case SDLK_4: makeHelix(verts, edges); currentShape=4; printf("Shape: Helix\n"); break;
                case SDLK_5: makeGrid(verts, edges); currentShape=5; printf("Shape: Grid\n"); break;
                }
            }

        }
        if (paused) { SDL_Delay(10); continue; }

        Uint32 nowTicks = SDL_GetTicks();
        float dt = (nowTicks - lastTicks) / 1000.0f; lastTicks = nowTicks;
        if (dt > 0.25f) dt = 0.25f; // clamp big spikes
        angle += dt;

        // render to our RGB buffer
        renderer.clear(10,10,30);
        std::vector<std::pair<int,int>> projected;
        projected.reserve(verts.size());
        for (auto &v : verts) {
            Vec3 r = rotateY(rotateX(v, angle*0.6f), angle);
            float z = r.z + cameraZ;
            float x2 = (r.x / z) * scale + W * 0.5f;
            float y2 = (r.y / z) * scale + H * 0.5f;
            projected.push_back({ int(x2 + 0.5f), int(y2 + 0.5f) });
        }
        for (auto &e : edges) {
            auto a = projected[e.first];
            auto b = projected[e.second];
            renderer.drawLine(a.first, a.second, b.first, b.second, 230, 230, 230);
        }

        // blit into SDL surface
        if (SDL_LockSurface(surface) == 0) {
            unsigned char *dst = (unsigned char*)surface->pixels;
            const unsigned char *src = renderer.getBuffer();
            int dstPitch = surface->pitch;
            int srcRowBytes = W * 3;
            int copyBytesPerRow = (srcRowBytes <= dstPitch) ? srcRowBytes : dstPitch;
            for (int y = 0; y < H; ++y) {
                memcpy(dst + y * dstPitch, src + y * srcRowBytes, copyBytesPerRow);
            }
            SDL_UnlockSurface(surface);
            SDL_UpdateWindowSurface(win);
        }

        // FPS calc + title
        framesThisSec++;
        auto now = std::chrono::steady_clock::now();
        double sec = std::chrono::duration<double>(now - t0).count();
        accumSec += sec;
        if (sec >= 1.0) {
            fps = framesThisSec / sec;
            framesThisSec = 0;
            t0 = now;
            char title[256];
            snprintf(title, sizeof(title), "Software Renderer (SDL2) — FPS: %.1f — Frame: %d", fps, frame);
            SDL_SetWindowTitle(win, title);
        }

        // Save single screenshot?
        if ((saveFrameOnStart && !savedStartFrame) || requestedSave) {
            char fname[256];
            snprintf(fname, sizeof(fname), "frame_%03d.ppm", frame);
            if (writePPM(fname, W, H, renderer.getBuffer()))
                printf("Saved %s\n", fname);
            savedStartFrame = true; requestedSave = false;
        }

        // Recording sequence?
        if (recordCount > 0 && recorded < recordCount) {
            // Ensure frames dir exists (race-safe)
            std::error_code ec;
            std::filesystem::create_directories(framesDir, ec);
            char fname[512];
            snprintf(fname, sizeof(fname), "%s/frame_%04d.ppm", framesDir.c_str(), recorded);
            if (writePPM(fname, W, H, renderer.getBuffer())) {
                if (recorded == 0) printf("Recording started -> %s\n", fname);
                if (recorded+1 == recordCount) printf("Recording finished: %d frames in '%s'\n", recordCount, framesDir.c_str());
            } else {
                fprintf(stderr, "Failed to write %s\n", fname);
            }
            recorded++;

            // pace recording to target fps
            if (targetFrameSec > 0.0) {
                // crude sleep-based pacing (good enough for capture)
                int ms = (int)std::round(targetFrameSec * 1000.0);
                if (ms > 0) SDL_Delay(ms);
            }
        } else {
            // normal run → small delay to be nice to CPU/GPU
            SDL_Delay(8);
        }

        frame++;
    }

    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
    printf("Program END\n");
    return 0;
}
