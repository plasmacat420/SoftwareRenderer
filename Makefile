# Makefile - MSYS2 MinGW64 using system SDL2 (Windows startup libs included)

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iinclude -I/mingw64/include -I/mingw64/include/SDL2 -Wall -Wextra
SOURCES = src/main.cpp src/renderer.cpp src/matrix4x4.cpp src/vector3D.cpp
TARGET = SoftwareRenderer.exe

# For MinGW-w64 + SDL2 we need the startup object/libs
# Order matters: put -lmingw32 and -lSDL2main before -lSDL2
SDL_LIBS = -lmingw32 -lSDL2main -lSDL2

all: $(TARGET)

# link step: put libs AFTER sources (order matters)
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SDL_LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.exe *.o frame_*.ppm
