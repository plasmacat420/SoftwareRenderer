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

// --- Ent/Tree ---
static void makeEnt(std::vector<Vec3> &verts, std::vector<Tri> &tris) {
    verts.clear(); tris.clear();
    // A simple trunk + leafy top
    verts = {
        {0,-1,0}, {0.3f,0,0}, {-0.3f,0,0}, {0,0,0.3f}, {0,0,-0.3f},
        {0,1,0}, {0.6f,1.3f,0}, {-0.6f,1.3f,0}, {0,1.3f,0.6f}, {0,1.3f,-0.6f}
    };
    tris = {
        {0,1,2,120,80,40},{0,2,3,120,80,40},{0,3,4,120,80,40},{0,4,1,120,80,40}, // trunk
        {1,5,2,120,80,40},{2,5,3,120,80,40},{3,5,4,120,80,40},{4,5,1,120,80,40}, // trunk top
        {5,6,7,30,120,30},{5,7,8,30,120,30},{5,8,9,30,120,30},{5,9,6,30,120,30}  // leafy top
    };
}

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
        switch(idx%5){
            case 0: makeCube(verts,tris); break;
            case 1: makeTetrahedron(verts,tris); break;
            case 2: makeIcosahedron(verts,tris); break;
            case 3: makeHelix(verts,tris); break;
            case 4: makeEnt(verts,tris); break;
        }
    };
    int shapeIndex=0; loadShape(shapeIndex);

    float cameraZ=3.5f, fov=90.0f;
    float scale=(1.0f/tanf((fov*0.5f)*PI/180.0f))*(W/2.0f);
    Vec3 lightDir={-1,0,-0.5f};
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
                    case SDLK_5: loadShape(shapeIndex=4); break;
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

        SDL_LockSurface(surface);
        memcpy(surface->pixels, renderer.getBuffer(), W*H*4);
        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(win);
        SDL_Delay(16);
    }
    SDL_DestroyWindow(win); SDL_Quit(); return 0;
}
