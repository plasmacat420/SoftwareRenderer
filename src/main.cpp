// src/main.cpp - Shape Shifter with extra high-graphic carrot shape (key 6)
#include "Renderer.h"
#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <string>
#include <cstdlib>

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

// --- Tri struct ---
struct Tri {
    int v0,v1,v2;
    unsigned char r,g,b;
};

// --- Shape generators ---
static void makeCube(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}
    };
    tris = {
        {0,1,2, 220,220,220},{0,2,3, 220,220,220},
        {4,6,5, 200,200,200},{4,7,6, 200,200,200},
        {0,5,1, 180,180,180},{0,4,5, 180,180,180},
        {2,6,7, 180,180,180},{2,7,3, 180,180,180},
        {1,5,6, 160,160,160},{1,6,2, 160,160,160},
        {0,3,7, 160,160,160},{0,7,4, 160,160,160}
    };
}

static void makeTetrahedron(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts = { {0,0,1.2f}, {1,0,-0.4f}, {-0.5f,0.87f,-0.4f}, {-0.5f,-0.87f,-0.4f} };
    tris = {
        {0,1,2, 220,180,180}, {0,2,3, 180,220,180},
        {0,3,1, 180,180,220}, {1,3,2, 220,220,180}
    };
}

static void makeIcosahedron(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts.clear(); tris.clear();
    float phi = (1 + sqrtf(5.0f)) * 0.5f;
    verts = {
        {-1,  phi, 0}, {1,  phi, 0}, {-1, -phi, 0}, {1, -phi, 0},
        {0, -1,  phi}, {0,  1,  phi}, {0, -1, -phi}, {0,  1, -phi},
        { phi, 0, -1}, { phi, 0,  1}, {-phi, 0, -1}, {-phi, 0,  1}
    };
    for (auto &v: verts) {
        float l = sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
        v.x/=l; v.y/=l; v.z/=l;
    }
    tris = {
        {0,11,5, 200,200,255},{0,5,1, 200,255,200},
        {0,1,7, 255,200,200},{0,7,10,220,220,180},
        {0,10,11,180,220,220}
    };
}

static void makeHelix(std::vector<Vec3> &verts, std::vector<Tri> &tris, int N=100) {
    verts.clear(); tris.clear();
    for (int i=0; i<N; i++) {
        float t=i*0.2f;
        verts.push_back({cosf(t), sinf(t), t*0.1f});
        if(i>=2) tris.push_back({i-2,i-1,i,200,180,255});
    }
}

// --- Ent/Tree (simple leafy top) ---
static void makeEnt(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts.clear(); tris.clear();
    verts = {
        {0,-1,0}, {0.3f,0,0}, {-0.3f,0,0}, {0,0,0.3f}, {0,0,-0.3f},
        {0,1,0}, {0.6f,1.3f,0}, {-0.6f,1.3f,0}, {0,1.3f,0.6f}, {0,1.3f,-0.6f}
    };
    tris = {
        {0,1,2,120,80,40},{0,2,3,120,80,40},{0,3,4,120,80,40},{0,4,1,120,80,40},
        {1,5,2,120,80,40},{2,5,3,120,80,40},{3,5,4,120,80,40},{4,5,1,120,80,40},
        {5,6,7,30,120,30},{5,7,8,30,120,30},{5,8,9,30,120,30},{5,9,6,30,120,30}
    };
}

// --- Carrot (high-graphic) generator ---
static void makeCarrot(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts.clear(); tris.clear();
    srand(424242); // deterministic

    const int segments = 28;   // around - fairly smooth
    const int rings = 18;      // along height
    const float baseY = -1.0f;
    const float topY  = 0.9f;

    std::vector<int> ringStart;
    ringStart.reserve(rings);

    for (int ri = 0; ri < rings; ++ri) {
        float t = (float)ri / (rings - 1);           // 0..1
        float y = baseY + t * (topY - baseY);
        // radius: large at base, small at top; add ridge noise
        float ridge = 0.06f * sinf(t * 18.0f + 0.5f * ((rand()%100)/100.0f));
        float radius = (1.0f - powf(t, 1.6f)) * 0.45f + ridge;
        // slight twist so it looks organic
        float twist = t * 2.0f * PI * 0.18f;

        int start = (int)verts.size();
        ringStart.push_back(start);

        for (int s = 0; s < segments; ++s) {
            float a = (float)s / segments * 2.0f * PI + twist;
            float x = cosf(a) * radius;
            float z = sinf(a) * radius;
            float wob = 0.02f * sinf(t * 10.0f + s * 0.5f);
            verts.push_back({ x + wob * cosf(a*2.3f), y + 0.01f * sinf(a*3.1f), z + wob * sinf(a*1.7f) });
        }
    }

    for (int ri = 1; ri < rings; ++ri) {
        int prev = ringStart[ri-1];
        int cur  = ringStart[ri];
        for (int s = 0; s < segments; ++s) {
            int a0 = prev + s;
            int a1 = prev + ((s+1) % segments);
            int b0 = cur  + s;
            int b1 = cur  + ((s+1) % segments);
            tris.push_back({ a0, a1, b1, 220,100,30 });
            tris.push_back({ a0, b1, b0, 200,90,20 });
        }
    }

    // tip
    Vec3 tipPos = { 0.0f, topY + 0.06f, 0.0f };
    int tipIndex = (int)verts.size();
    verts.push_back(tipPos);

    int lastStart = ringStart.back();
    for (int s = 0; s < segments; ++s) {
        int v0 = lastStart + s;
        int v1 = lastStart + ((s+1) % segments);
        tris.push_back({ v0, v1, tipIndex, 230,110,40 });
    }

    // leafy tuft
    int leafCenter = (int)verts.size();
    verts.push_back({ 0.0f, topY + 0.10f, 0.0f }); // leaf center

    int leafCount = 8;
    for (int i = 0; i < leafCount; ++i) {
        float a = (float)i / leafCount * 2.0f * PI;
        float lx = cosf(a) * 0.20f;
        float lz = sinf(a) * 0.20f;
        float ly = topY + 0.10f + 0.03f * cosf(a*2.0f);
        int leafOuter = (int)verts.size();
        verts.push_back({ lx * 0.6f, ly - 0.03f, lz * 0.6f });
        int leafTip = (int)verts.size();
        verts.push_back({ lx * 1.1f, ly + 0.02f, lz * 1.1f });
        unsigned char gr = (unsigned char)(30 + (rand()%60));   // 30..89
        unsigned char gg = (unsigned char)(110 + (rand()%80));  // 110..189
        unsigned char gb = (unsigned char)(20 + (rand()%40));
        tris.push_back({ leafCenter, leafOuter, leafTip, gr, gg, gb });
    }

    // freckles / small details at lower half
    for (int f = 0; f < 12; ++f) {
        float ty = baseY + ((float)rand()/RAND_MAX) * (topY - baseY) * 0.45f;
        float ta = ((float)rand()/RAND_MAX) * 2.0f * PI;
        float tr = 0.02f + ((float)rand()/RAND_MAX) * 0.03f;
        Vec3 p0 = { cosf(ta)*tr*0.3f, ty, sinf(ta)*tr*0.3f };
        Vec3 p1 = { cosf(ta+0.3f)*tr, ty+0.01f, sinf(ta+0.3f)*tr };
        Vec3 p2 = { cosf(ta-0.3f)*tr, ty-0.01f, sinf(ta-0.3f)*tr };
        int i0 = (int)verts.size(); verts.push_back(p0);
        int i1 = (int)verts.size(); verts.push_back(p1);
        int i2 = (int)verts.size(); verts.push_back(p2);
        tris.push_back({ i0, i1, i2, 160,70,30 });
    }
}

// --- main ---
int main(int argc, char** argv) {
    (void)argc; (void)argv;
    int W=800, H=600;

    if(SDL_Init(SDL_INIT_VIDEO)!=0) return 1;
    SDL_Window *win = SDL_CreateWindow("Shape Shifter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W,H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Surface *surface = SDL_GetWindowSurface(win);
    if(!surface){ SDL_Quit(); return 1; }

    W=surface->w; H=surface->h;
    Renderer renderer(W,H);

    std::vector<Vec3> verts; std::vector<Tri> tris;
    auto loadShape=[&](int idx){
        switch(idx%6){ // now 6 shapes: 0..5
            case 0: makeCube(verts,tris); break;
            case 1: makeTetrahedron(verts,tris); break;
            case 2: makeIcosahedron(verts,tris); break;
            case 3: makeHelix(verts,tris); break;
            case 4: makeEnt(verts,tris); break;
            case 5: makeCarrot(verts,tris); break;
        }
    };
    int shapeIndex=0; loadShape(shapeIndex);

    float cameraZ=3.5f, fov=90.0f;
    float scale=(1.0f/tanf((fov*0.5f)*PI/180.0f))*(W/2.0f);
    // Vec3 lightDir={-1,1.0f,-0.5f};
    // Vec3 lightDir = {1, 0.7f, 0.7f};
    float t = SDL_GetTicks() * 0.001f; // seconds
    Vec3 lightDir = {cosf(t)*1, 0.7f, sinf(t)*0.7f};
    float len=sqrtf(lightDir.x*lightDir.x+lightDir.y*lightDir.y+lightDir.z*lightDir.z);
    lightDir.x/=len; lightDir.y/=len; lightDir.z/=len;

    float angle=0; bool running=true; SDL_Event ev;
    while(running){
        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT) running=false;
            if(ev.type==SDL_KEYDOWN){
                switch(ev.key.keysym.sym){
                    case SDLK_ESCAPE: running=false; break;
                    case SDLK_1: loadShape(shapeIndex=0); break;
                    case SDLK_2: loadShape(shapeIndex=1); break;
                    case SDLK_3: loadShape(shapeIndex=2); break;
                    case SDLK_4: loadShape(shapeIndex=3); break;
                    case SDLK_5: loadShape(shapeIndex=4); break; // Ent
                    case SDLK_6: loadShape(shapeIndex=5); break; // Carrot
                }
            }
        }
        angle+=0.01f;
        renderer.clear(10,10,30);
        renderer.clearZ();

        for(auto &t: tris){
            Vec3 av=verts[t.v0], bv=verts[t.v1], cv=verts[t.v2];
            av=rotateY(rotateX(av,angle*0.6f),angle);
            bv=rotateY(rotateX(bv,angle*0.6f),angle);
            cv=rotateY(rotateX(cv,angle*0.6f),angle);

            Vec3 ab={bv.x-av.x,bv.y-av.y,bv.z-av.z};
            Vec3 ac={cv.x-av.x,cv.y-av.y,cv.z-av.z};
            Vec3 normal={ab.y*ac.z-ab.z*ac.y,ab.z*ac.x-ab.x*ac.z,ab.x*ac.y-ab.y*ac.x};
            float nlen=sqrtf(normal.x*normal.x+normal.y*normal.y+normal.z*normal.z);
            if(nlen>1e-6f){normal.x/=nlen;normal.y/=nlen;normal.z/=nlen;}
            float brightness=normal.x*lightDir.x+normal.y*lightDir.y+normal.z*lightDir.z;
            if(brightness<0) brightness=0;

            auto project=[&](Vec3 v){
                float z=v.z+cameraZ;
                return Vec3{(v.x/z)*scale+W*0.5f,(v.y/z)*scale+H*0.5f,z};
            };
            Vec3 a=project(av), b=project(bv), c=project(cv);
            renderer.drawTriangle((int)a.x,(int)a.y,a.z,(int)b.x,(int)b.y,b.z,(int)c.x,(int)c.y,c.z,t.r,t.g,t.b,brightness);
        }

        // copy ARGB32 buffer exactly (W*H*4)
        if (SDL_LockSurface(surface) == 0) {
            unsigned char *dst = (unsigned char*)surface->pixels;
            const unsigned char *src = renderer.getBuffer();
            int dstPitch = surface->pitch;
            int rowBytes = W * 4;
            int copyBytes = (rowBytes <= dstPitch) ? rowBytes : dstPitch;
            for (int y=0; y<H; ++y) {
                memcpy(dst + y * dstPitch, src + y * rowBytes, copyBytes);
            }
            SDL_UnlockSurface(surface);
            SDL_UpdateWindowSurface(win);
        }

        SDL_Delay(16);
    }
    SDL_DestroyWindow(win); SDL_Quit(); return 0;
}
