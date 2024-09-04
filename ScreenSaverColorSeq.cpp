#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <string>

const int CELL_SIZE = 6;  // Puedes ajustar esto también si lo deseas parametrizar

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;
    int frameCount;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    float fps;
    int numObjects;
    int screenWidth;
    int screenHeight;
    int gridWidth;
    int gridHeight;
    int frameDelay;

public:
    Game(int objects, int width, int height)
        : window(nullptr), renderer(nullptr), texture(nullptr), frameCount(0), fps(0),
          numObjects(objects), screenWidth(width), screenHeight(height) {
        gridWidth = screenWidth / CELL_SIZE;
        gridHeight = screenHeight / CELL_SIZE;
        frameDelay = 1000 / 60;  // TARGET_FPS = 60

        grid.resize(gridHeight, std::vector<bool>(gridWidth, false));
        nextGrid.resize(gridHeight, std::vector<bool>(gridWidth, false));
        lastTime = std::chrono::high_resolution_clock::now();
    }

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Error al iniciar SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Error al crear ventana: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Error al crear renderer: " << SDL_GetError() << std::endl;
            return false;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gridWidth, gridHeight);
        if (!texture) {
            std::cerr << "Error al crear textura: " << SDL_GetError() << std::endl;
            return false;
        }

        std::cout << "Inicialización completada" << std::endl;
        return true;
    }

    void updateWindowTitle() {
        std::string title = "Conway's Game of Life - FPS: " + std::to_string(static_cast<int>(fps));
        SDL_SetWindowTitle(window, title.c_str());
    }

    void calculateFPS() {
        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

        if (duration > 1.0f) {
            fps = frameCount / duration;
            frameCount = 0;
            lastTime = currentTime;
            updateWindowTitle();
        }
    }

    void placePattern(int x, int y, const std::vector<std::vector<int>>& pattern) {
        for (int i = 0; i < pattern.size(); ++i) {
            for (int j = 0; j < pattern[i].size(); ++j) {
                if (pattern[i][j] == 1) {
                    grid[(y + i) % gridHeight][(x + j) % gridWidth] = true;
                }
            }
        }
    }

    void generateFigures() {
        std::vector<std::vector<std::vector<int>>> patterns = {
            {{0, 1, 0}, {0, 0, 1}, {1, 1, 1}},  // Glider
            {{1, 1}, {1, 1}},                  // Block
            {{1, 1, 1}},                       // Blinker
            {{0, 1, 1, 1}, {1, 1, 1, 0}},      // Toad
            {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 1, 1}, {0, 0, 1, 1}}, // Beacon
            {
                {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0}, 
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
                {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1}, 
                {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, 
                {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0}
            },  // Pulsar
            {{0, 1, 1, 1, 1}, {1, 0, 0, 0, 1}}, // LWSS (Lightweight Spaceship)
            {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}    // Pentadecathlon
        };

        srand(time(nullptr));

        auto start = std::chrono::high_resolution_clock::now(); // Iniciar medición de tiempo

        for (int i = 0; i < numObjects; ++i) {
            int patternIndex = rand() % patterns.size();
            int x = rand() % gridWidth;
            int y = rand() % gridHeight;
            placePattern(x, y, patterns[patternIndex]);
        }

        auto end = std::chrono::high_resolution_clock::now(); // Fin de medición de tiempo
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tiempo para generar figuras: " << duration.count() << " segundos" << std::endl;
    }

    void randomizeGrid() {
        srand(time(nullptr));
        int objectsPlaced = 0;
        auto start = std::chrono::high_resolution_clock::now();

        while (objectsPlaced < numObjects) {
            int x = rand() % gridWidth;
            int y = rand() % gridHeight;
            if (!grid[y][x]) {
                grid[y][x] = true;
                objectsPlaced++;
            }

            auto current = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = current - start;

            if (elapsed.count() > frameDelay) {
                SDL_Delay(frameDelay - elapsed.count());
                calculateFPS();
                start = std::chrono::high_resolution_clock::now();
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tiempo para generar " << numObjects << " elementos: " << duration.count() << " segundos" << std::endl;
    }

    int countNeighbors(int x, int y) {
        int count = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx + gridWidth) % gridWidth;
                int ny = (y + dy + gridHeight) % gridHeight;
                count += grid[ny][nx];
            }
        }
        return count;
    }

    void update() {
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                int neighbors = countNeighbors(x, y);
                nextGrid[y][x] = (grid[y][x] && (neighbors == 2 || neighbors == 3)) || (!grid[y][x] && neighbors == 3);
            }
        }
        std::swap(grid, nextGrid);
    }

    void render() {
      if (!renderer || !texture) {
          std::cerr << "Error: Renderer o textura no inicializados correctamente." << std::endl;
          return;
      }

      void* pixels;
      int pitch;
      if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
          std::cerr << "Error al bloquear la textura: " << SDL_GetError() << std::endl;
          return;
      }

      Uint32* pixelData = static_cast<Uint32*>(pixels);

      for (int y = 0; y < gridHeight; ++y) {
          for (int x = 0; x < gridWidth; ++x) {
              if (grid[y][x]) {
                  // Asignar color aleatorio a las células vivas
                  Uint8 r = rand() % 256;
                  Uint8 g = rand() % 256;
                  Uint8 b = rand() % 256;
                  pixelData[y * gridWidth + x] = (r << 24) | (g << 16) | (b << 8) | 0xFF;  // Color RGBA
              } else {
                  // Células muertas en negro
                  pixelData[y * gridWidth + x] = 0x000000FF;  // Negro
              }
          }
      }

      SDL_UnlockTexture(texture);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, nullptr, nullptr);
      SDL_RenderPresent(renderer);
  }


    void run() {
        generateFigures(); // Generar figuras predefinidas

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            auto frameStart = std::chrono::high_resolution_clock::now();

            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }

            update();
            render();
            calculateFPS();

            auto frameEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> frameDuration = frameEnd - frameStart;
            int delay = frameDelay - static_cast<int>(frameDuration.count());
            if (delay > 0) {
                SDL_Delay(delay);
            }
        }
    }

    void close() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        TTF_Quit();
        SDL_Quit();
    }
};

int main(int argc, char* args[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << args[0] << " <número de objetos> <ancho de pantalla> <alto de pantalla>" << std::endl;
        return 1;
    }

    char* end;
    long numObjects = std::strtol(args[1], &end, 10);
    if (*end != '\0' || numObjects <= 0) {
        std::cerr << "El número de objetos debe ser un entero positivo." << std::endl;
        return 1;
    }

    long screenWidth = std::strtol(args[2], &end, 10);
    if (*end != '\0' || screenWidth <= 0) {
        std::cerr << "El ancho de pantalla debe ser un entero positivo." << std::endl;
        return 1;
    }

    long screenHeight = std::strtol(args[3], &end, 10);
    if (*end != '\0' || screenHeight <= 0) {
        std::cerr << "El alto de pantalla debe ser un entero positivo." << std::endl;
        return 1;
    }

    Game game(static_cast<int>(numObjects), static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    if (!game.init()) {
        game.close();
        return 1;
    }
    game.run();
    game.close();
    return 0;
}
