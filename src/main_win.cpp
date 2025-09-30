#include "Renderer.h"
#include <windows.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdio>

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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    const int W = 640;
    const int H = 480;
    const char *wndclass = "SoftwareRendererWinClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = wndclass;
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, wndclass, "Software Renderer (Win32)",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, // leave room for window chrome
        NULL, NULL, wc.hInstance, NULL
    );
    if (!hwnd) return 1;

    // Prepare DIB info for 24-bit RGB (top-down)
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = -H; // negative for top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    Renderer renderer(W, H);

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

    // make window respond to messages while we render in the loop
    ShowWindow(hwnd, SW_SHOW);

    bool running = true;
    auto last = std::chrono::steady_clock::now();
    float angle = 0.0f;

    // set up a simple message handler that will allow closing the window
    // by responding to WM_CLOSE -> PostQuitMessage
    SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR) + [](HWND h, UINT msg, WPARAM w, LPARAM l) -> LRESULT {
        if (msg == WM_CLOSE) { PostQuitMessage(0); return 0; }
        return DefWindowProcA(h,msg,w,l);
    });

    // main loop
    while (running) {
        // process Windows messages
        MSG msg;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!running) break;

        // timing
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - last;
        last = now;
        float dt = delta.count();
        angle += dt;

        // render into software buffer
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

        // blit software buffer to window
        HDC hdc = GetDC(hwnd);
        const unsigned char *buf = renderer.getBuffer(); // RGB24 top->bottom
        // StretchDIBits expects BGR on some systems; our buffer is RGB. Many Windows devices expect BGR,
        // but StretchDIBits with DIB_RGB_COLORS will put the bytes as we give them. If colors look swapped,
        // we can convert, but try direct copy first.
        StretchDIBits(hdc, 0, 0, W, H, 0, 0, W, H, buf, &bmi, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(hwnd, hdc);

        // simple 60 FPS cap
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
