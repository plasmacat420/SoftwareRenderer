// src/renderer.cpp
#include "Renderer.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cmath>

Renderer::Renderer(int w, int h) : width(w), height(h) {
    buffer = new unsigned char[width * height * 3];
    zbuffer.resize(w*h, 1e9f);
    clear(0,0,0);
}
void Renderer::clearZ() {
    std::fill(zbuffer.begin(), zbuffer.end(), 1e9f);
}
Renderer::~Renderer() {
    delete[] buffer;
}

void Renderer::clear(unsigned char r, unsigned char g, unsigned char b) {
    for (int i = 0; i < width * height; ++i) {
        buffer[i*3 + 0] = r;
        buffer[i*3 + 1] = g;
        buffer[i*3 + 2] = b;
    }
}

void Renderer::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    int idx = (y * width + x) * 3;
    buffer[idx + 0] = r;
    buffer[idx + 1] = g;
    buffer[idx + 2] = b;
}

void Renderer::drawLine(int x0, int y0, int x1, int y1,
                        unsigned char r, unsigned char g, unsigned char b) {
    // Bresenham line algorithm (handles steep lines)
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) std::swap(x0, y0), std::swap(x1, y1);
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;
    for (int x = x0; x <= x1; ++x) {
        if (steep) setPixel(y, x, r, g, b);
        else setPixel(x, y, r, g, b);
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}
void Renderer::drawTriangle(int x0,int y0,float z0,
                            int x1,int y1,float z1,
                            int x2,int y2,float z2,
                            unsigned char r, unsigned char g, unsigned char b,
                            float intensity) {
    // clamp intensity
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;

    // shade base color
    unsigned char R = (unsigned char)(r * intensity);
    unsigned char G = (unsigned char)(g * intensity);
    unsigned char B = (unsigned char)(b * intensity);

    // bounding box
    int minX = std::max(0, std::min({x0,x1,x2}));
    int maxX = std::min(width-1, std::max({x0,x1,x2}));
    int minY = std::max(0, std::min({y0,y1,y2}));
    int maxY = std::min(height-1, std::max({y0,y1,y2}));

    // edge function
    auto edge = [](int x0,int y0,int x1,int y1,int x,int y){
        return (x - x0)*(y1 - y0) - (y - y0)*(x1 - x0);
    };

    float area = (float)edge(x0,y0, x1,y1, x2,y2);
    if (area == 0) return;

    // scan pixels
    for (int y=minY; y<=maxY; y++) {
        for (int x=minX; x<=maxX; x++) {
            float w0 = (float)edge(x1,y1,x2,y2,x,y);
            float w1 = (float)edge(x2,y2,x0,y0,x,y);
            float w2 = (float)edge(x0,y0,x1,y1,x,y);

            if ((w0>=0 && w1>=0 && w2>=0) || (w0<=0 && w1<=0 && w2<=0)) {
                // barycentric (normalize)
                w0 /= area; w1 /= area; w2 /= area;
                float z = w0*z0 + w1*z1 + w2*z2;
                int idx = y*width + x;
                if (z < zbuffer[idx]) {
                    zbuffer[idx] = z;
                    int i = idx*3;
                    buffer[i+0] = R;
                    buffer[i+1] = G;
                    buffer[i+2] = B;
                }
            }
        }
    }
}


void Renderer::present(const std::string &filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return;
    // P6 PPM header
    ofs << "P6\n" << width << " " << height << "\n255\n";
    ofs.write(reinterpret_cast<char*>(buffer), width * height * 3);
    ofs.close();
}
