// src/main.cpp - SDL2 renderer with triangle support
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

// --- Triangle structure ---
struct Tri {
    int v0, v1, v2;
    unsigned char r, g, b;

    Tri(int a, int b, int c, unsigned char R, unsigned char G, unsigned char B)
        : v0(a), v1(b), v2(c), r(R), g(G), b(B) {}
};

// --- Shape generators ---
static void makeCubeMesh(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };
    tris.clear();
    // back
    tris.emplace_back(0,1,2, 220,220,220);
    tris.emplace_back(0,2,3, 220,220,220);
    // front
    tris.emplace_back(4,6,5, 200,200,200);
    tris.emplace_back(4,7,6, 200,200,200);
    // left
    tris.emplace_back(0,5,1, 180,180,180);
    tris.emplace_back(0,4,5, 180,180,180);
    // right
    tris.emplace_back(2,6,7, 180,180,180);
    tris.emplace_back(2,7,3, 180,180,180);
    // top
    tris.emplace_back(1,5,6, 160,160,160);
    tris.emplace_back(1,6,2, 160,160,160);
    // bottom
    tris.emplace_back(0,3,7, 160,160,160);
    tris.emplace_back(0,7,4, 160,160,160);
}

// --- Simple PPM writer ---
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
    // CLI options
    int W = 640, H = 480;
    int recordCount = 0, recordFps = 30;
    std::string framesDir = "frames";

    for (int i=1; i<argc; ++i) {
        std::string s(argv[i]);
        if (s=="--width" && i+1<argc)  W = std::max(64, atoi(argv[++i]));
        else if (s=="--height" && i+1<argc) H = std::max(64, atoi(argv[++i]));
        else if (s=="--record" && i+1<argc) recordCount = std::max(0, atoi(argv[++i]));
        else if (s=="--record-fps" && i+1<argc) recordFps = std::max(1, atoi(argv[++i]));
        else if (s=="--frames-dir" && i+1<argc) framesDir = argv[++i];
    }

    printf("PROGRAM START (W=%d H=%d)\n", W,H);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: '%s'\n", SDL_GetError()); return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Software Renderer (Triangles)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    if (!win) { fprintf(stderr, "SDL_CreateWindow failed: '%s'\n", SDL_GetError()); SDL_Quit(); return 1; }

    SDL_Surface *surface = SDL_GetWindowSurface(win);
    if (!surface) { fprintf(stderr, "SDL_GetWindowSurface failed\n"); SDL_DestroyWindow(win); SDL_Quit(); return 1; }

    Renderer renderer(W,H);

    std::vector<Vec3> verts;
    std::vector<Tri> tris;
    makeCubeMesh(verts, tris);

    const float cameraZ = 4.0f;
    const float fov = 90.0f;
    float f = 1.0f / tanf((fov * 0.5f) * PI / 180.0f);
    float scale = f * (W / 2.0f);

    bool running = true;
    SDL_Event ev;
    Uint32 lastTicks = SDL_GetTicks();
    float angle = 0.0f;
    int frame = 0;
    int recorded = 0;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        Uint32 nowTicks = SDL_GetTicks();
        float dt = (nowTicks - lastTicks) / 1000.0f; lastTicks = nowTicks;
        angle += dt;

        renderer.clear(20,20,40);
        renderer.clearZ();

        // transform + project
        std::vector<Vec3> projectedVerts;
        projectedVerts.reserve(verts.size());
        for (auto &v : verts) {
            Vec3 r = rotateY(rotateX(v, angle*0.7f), angle*0.3f);
            float z = r.z + cameraZ;
            float x2 = (r.x / z) * scale + W * 0.5f;
            float y2 = (r.y / z) * scale + H * 0.5f;
            projectedVerts.push_back({x2,y2,z});
        }

        // draw triangles
        // simple directional light (normalized)
        Vec3 lightDir = {-1, 0, 0};
        {
            float len = sqrtf(lightDir.x*lightDir.x + lightDir.y*lightDir.y + lightDir.z*lightDir.z);
            lightDir.x /= len; lightDir.y /= len; lightDir.z /= len;
        }

        for (auto &t : tris) {
            Vec3 av = verts[t.v0];
            Vec3 bv = verts[t.v1];
            Vec3 cv = verts[t.v2];

            // rotate
            av = rotateY(rotateX(av, angle*0.6f), angle);
            bv = rotateY(rotateX(bv, angle*0.6f), angle);
            cv = rotateY(rotateX(cv, angle*0.6f), angle);

            // compute normal
            Vec3 ab = {bv.x - av.x, bv.y - av.y, bv.z - av.z};
            Vec3 ac = {cv.x - av.x, cv.y - av.y, cv.z - av.z};
            Vec3 normal = {
                ab.y*ac.z - ab.z*ac.y,
                ab.z*ac.x - ab.x*ac.z,
                ab.x*ac.y - ab.y*ac.x
            };
            float nlen = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
            if (nlen > 1e-6f) {
                normal.x /= nlen; normal.y /= nlen; normal.z /= nlen;
            }

            float brightness = normal.x*lightDir.x + normal.y*lightDir.y + normal.z*lightDir.z;
            if (brightness < 0) brightness = 0;

            // projection lambda
            auto project = [&](Vec3 v) {
                float z = v.z + cameraZ;
                float x2 = (v.x / z) * scale + W * 0.5f;
                float y2 = (v.y / z) * scale + H * 0.5f;
                return Vec3{x2,y2,z};
            };

            Vec3 a = project(av);
            Vec3 b = project(bv);
            Vec3 c = project(cv);

            renderer.drawTriangle(
                (int)a.x,(int)a.y,a.z,
                (int)b.x,(int)b.y,b.z,
                (int)c.x,(int)c.y,c.z,
                t.r,t.g,t.b,
                brightness
            );
        }


        // for (auto &t : tris) {
        //     Vec3 a = projectedVerts[t.v0];
        //     Vec3 b = projectedVerts[t.v1];
        //     Vec3 c = projectedVerts[t.v2];
        //     renderer.drawTriangle(
        //         (int)a.x,(int)a.y,a.z,
        //         (int)b.x,(int)b.y,b.z,
        //         (int)c.x,(int)c.y,c.z,
        //         t.r,t.g,t.b,
        //         1.0f
        //     );
        // }

        // blit into SDL surface
        if (SDL_LockSurface(surface) == 0) {
            unsigned char *dst = (unsigned char*)surface->pixels;
            const unsigned char *src = renderer.getBuffer();
            int dstPitch = surface->pitch;
            int srcRowBytes = W * 3;
            for (int y = 0; y < H; ++y) {
                memcpy(dst + y * dstPitch, src + y * srcRowBytes, srcRowBytes);
            }
            SDL_UnlockSurface(surface);
            SDL_UpdateWindowSurface(win);
        }

        // recording
        if (recordCount > 0 && recorded < recordCount) {
            std::filesystem::create_directories(framesDir);
            char fname[256];
            snprintf(fname, sizeof(fname), "%s/frame_%04d.ppm", framesDir.c_str(), recorded);
            writePPM(fname, W, H, renderer.getBuffer());
            recorded++;
            SDL_Delay(1000 / recordFps);
        } else {
            SDL_Delay(16);
        }

        frame++;
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    printf("Program END\n");
    return 0;
}
