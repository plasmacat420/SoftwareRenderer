#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    // Clear color (r,g,b) 0-255
    void clear(unsigned char r, unsigned char g, unsigned char b);
    // Reset Z buffer
    void clearZ();

    // Set a single pixel (z used for z-buffer test)
    void setPixel(int x, int y, float z, unsigned char r, unsigned char g, unsigned char b);

    // Draw a filled triangle with simple z-interpolation and flat brightness
    void drawTriangle(
        int x0,int y0,float z0,
        int x1,int y1,float z1,
        int x2,int y2,float z2,
        unsigned char r,unsigned char g,unsigned char b,
        float brightness
    );

    // Raw buffer bytes (ARGB32, little-endian: 0xAARRGGBB). Returned as byte pointer.
    const unsigned char* getBuffer() const { return reinterpret_cast<const unsigned char*>(buffer); }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width, height;
    std::vector<float> zbuffer;
    uint32_t* buffer;  // ARGB32 pixel buffer (row-major)
};
