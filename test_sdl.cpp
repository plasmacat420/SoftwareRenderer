#include <SDL3/SDL.h>
#include <iostream>
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: '" << SDL_GetError() << "'\n";
        return 1;
    }
    std::cout << "SDL_Init OK\n";
    SDL_Quit();
    return 0;
}
