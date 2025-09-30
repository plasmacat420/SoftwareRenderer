#include "Renderer.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cmath>

Renderer::Renderer(int w, int h) : width(w), height(h) {
    buffer = new unsigned char[width * height * 3];
    clear(0,0,0);
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

void Renderer::present(const std::string &filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return;
    // P6 PPM header
    ofs << "P6\n" << width << " " << height << "\n255\n";
    ofs.write(reinterpret_cast<char*>(buffer), width * height * 3);
    ofs.close();
}
