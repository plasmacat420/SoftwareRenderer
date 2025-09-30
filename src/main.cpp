// src/main.cpp - SDL2-only version

#include "Renderer.h"
#include <SDL2/SDL.h>

#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>

struct Vec3 { float x,y,z; };
constexpr float PI = 3.14159265358979323846f;

Vec3 rotateY(const Vec3 &v, float a){
    float s = sinf(a), c = cosf(a);
    return { c*v.x + s*v.z, v.y, -s*v.x + c*v.z };
}
Vec3 rotateX(const Vec3 &v, float a){
    float s = sinf(a), c = cosf(a);
    return { v.x, c*v.y - s*v.z, s*v.y + c*v.z };
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    const int W = 640;
    const int H = 480;
    printf("PROGRAM START\n"); fflush(stdout);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: '%s'\n", SDL_GetError()); fflush(stdout);
        return 1;
    }
    printf("SDL_Init OK\n"); fflush(stdout);

    SDL_Window *win = SDL_CreateWindow(
        "Software Renderer (SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        W, H, 0
    );
    if (!win) {
        printf("SDL_CreateWindow failed: '%s'\n", SDL_GetError()); fflush(stdout);
        SDL_Quit();
        return 1;
    }
    printf("SDL_CreateWindow OK (win=%p)\n", (void*)win); fflush(stdout);

    SDL_Surface *surface = SDL_GetWindowSurface(win);
    if (!surface) {
        printf("SDL_GetWindowSurface failed: '%s'\n", SDL_GetError()); fflush(stdout);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    printf("SDL_GetWindowSurface OK (surface=%p pitch=%d)\n", (void*)surface, surface->pitch); fflush(stdout);

    Renderer renderer(W, H);
    printf("Renderer constructed\n"); fflush(stdout);

    std::vector<Vec3> verts = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };
    std::vector<std::pair<int,int>> edges = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    const float cameraZ = 4.0f;
    const float fov = 90.0f;
    float f = 1.0f / tanf((fov * 0.5f) * PI / 180.0f);
    float scale = f * (W / 2.0f);

    printf("Entering main loop\n"); fflush(stdout);

    bool running = true;
    SDL_Event ev;
    Uint32 last = SDL_GetTicks();
    float angle = 0.0f;
    int frame = 0;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
        }

        const Uint8 *keyState = SDL_GetKeyboardState(NULL);
        if (keyState && keyState[SDL_SCANCODE_ESCAPE]) running = false;

        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        angle += dt;

        // render
        renderer.clear(10,10,30);
        std::vector<std::pair<int,int>> projected;
        projected.reserve(verts.size());
        for (auto &v : verts) {
            Vec3 r = rotateY(v, angle);
            r = rotateX(r, angle * 0.6f);
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
        } else {
            printf("SDL_LockSurface failed: '%s'\n", SDL_GetError()); fflush(stdout);
        }

        SDL_Delay(16);
        frame++;
        if (frame == 1) { printf("Frame 1 drawn\n"); fflush(stdout); } // quick checkpoint
    }

    printf("Exiting loop, cleaning up\n"); fflush(stdout);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
    printf("Program END\n"); fflush(stdout);
    return 0;
}
