#include "Renderer.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>

// pack RGB * brightness into 0xAARRGGBB (alpha = 0xFF)
static inline uint32_t packColor(unsigned char r, unsigned char g, unsigned char b, float brightness) {
    int ri = std::min(255, std::max(0, int(std::round(r * brightness))));
    int gi = std::min(255, std::max(0, int(std::round(g * brightness))));
    int bi = std::min(255, std::max(0, int(std::round(b * brightness))));
    return (static_cast<uint32_t>(0xFF) << 24) |
           (static_cast<uint32_t>(ri) << 16) |
           (static_cast<uint32_t>(gi) << 8) |
            static_cast<uint32_t>(bi);
}

Renderer::Renderer(int w, int h) : width(w), height(h) {
    buffer = new uint32_t[width * height];
    zbuffer.resize(width * height, 1e9f);
    clear(0,0,0);
}
Renderer::~Renderer() {
    delete[] buffer;
}

void Renderer::clear(unsigned char r, unsigned char g, unsigned char b) {
    uint32_t color = packColor(r,g,b,1.0f);
    for (int i = 0; i < width * height; ++i) buffer[i] = color;
}

void Renderer::clearZ() {
    std::fill(zbuffer.begin(), zbuffer.end(), 1e9f);
}

void Renderer::setPixel(int x, int y, float z, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    int idx = y * width + x;
    if (z < zbuffer[idx]) {
        zbuffer[idx] = z;
        buffer[idx] = packColor(r,g,b,1.0f);
    }
}

// Simple barycentric triangle rasterizer (flat brightness)
void Renderer::drawTriangle(
    int x0,int y0,float z0,
    int x1,int y1,float z1,
    int x2,int y2,float z2,
    unsigned char r,unsigned char g,unsigned char b,
    float brightness
) {
    int minX = std::max(0, std::min({x0, x1, x2}));
    int maxX = std::min(width - 1, std::max({x0, x1, x2}));
    int minY = std::max(0, std::min({y0, y1, y2}));
    int maxY = std::min(height - 1, std::max({y0, y1, y2}));

    // Compute denominator for barycentric coordinates
    float denom = float((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
    if (std::fabs(denom) < 1e-6f) return;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float w0 = float((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / denom;
            float w1 = float((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / denom;
            float w2 = 1.0f - w0 - w1;
            if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                float z = w0 * z0 + w1 * z1 + w2 * z2;
                int idx = y * width + x;
                if (z < zbuffer[idx]) {
                    zbuffer[idx] = z;
                    buffer[idx] = packColor(r, g, b, brightness);
                }
            }
        }
    }
}
