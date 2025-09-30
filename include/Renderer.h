#pragma once
#include <string>
#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void clear(unsigned char r, unsigned char g, unsigned char b);
    void clearZ();

    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
    void drawLine(int x0, int y0, int x1, int y1,
                  unsigned char r, unsigned char g, unsigned char b);

    // NEW: draw filled triangle with depth + shading
    void drawTriangle(int x0,int y0,float z0,
                      int x1,int y1,float z1,
                      int x2,int y2,float z2,
                      unsigned char r, unsigned char g, unsigned char b,
                      float intensity);

    void present(const std::string &filename);

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    const unsigned char* getBuffer() const { return buffer; }

private:
    int width;
    int height;
    std::vector<float> zbuffer;
    unsigned char *buffer; // RGB triplets, row-major
};
