#pragma once
#include <string>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void clear(unsigned char r, unsigned char g, unsigned char b);
    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
    void drawLine(int x0, int y0, int x1, int y1, unsigned char r, unsigned char g, unsigned char b);
    void present(const std::string &filename);

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // read-only raw RGB24 buffer (row-major, width*height*3 bytes)
    const unsigned char* getBuffer() const { return buffer; }

private:
    int width;
    int height;
    unsigned char *buffer; // RGB triplets, row-major
};
