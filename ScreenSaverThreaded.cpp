#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <future>
#include <mutex>
#include <array>

const int SCREEN_WIDTH = 1840;
const int SCREEN_HEIGHT = 1155;
const int CELL_SIZE = 6;
const int GRID_WIDTH = SCREEN_WIDTH / CELL_SIZE;
const int GRID_HEIGHT = SCREEN_HEIGHT / CELL_SIZE;
const int TARGET_FPS = 60;
const int FRAME_DELAY = 1000 / TARGET_FPS;
const int NUM_COLORS = 10; // Número de colores en la paleta

std::array<Uint32, NUM_COLORS> colorPalette;

void initializeColorPalette() {
    srand(time(nullptr));
    for (int i = 0; i < NUM_COLORS; ++i) {
        // Generar colores aleatorios
        Uint8 r = rand() % 256;
        Uint8 g = rand() % 256;
        Uint8 b = rand() % 256;
        colorPalette[i] = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), r, g, b);
    }
}

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;
    std::vector<std::vector<int>> colorGrid; // Matriz para almacenar los colores de las células
    int frameCount;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    float fps;
    int numObjects;
    std::mutex mtx;  // Mutex para sincronización

    void updateBlock(int startY, int endY) {
        for (int y = startY; y < endY; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                int neighbors = countNeighbors(x, y);
                nextGrid[y][x] = (grid[y][x] && (neighbors == 2 || neighbors == 3)) || (!grid[y][x] && neighbors == 3);
            }
        }
    }

    void renderBlock(int startY, int endY, Uint32* pixelData) {
        for (int y = startY; y < endY; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                pixelData[y * GRID_WIDTH + x] = grid[y][x] ? colorPalette[colorGrid[y][x]] : 0x000000FF;
            }
        }
    }

public:
    Game(int objects) : window(nullptr), renderer(nullptr), texture(nullptr), frameCount(0), fps(0), numObjects(objects) {
        grid.resize(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
        nextGrid.resize(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
        colorGrid.resize(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0)); // Inicializar con color 0 (el primer color)
        lastTime = std::chrono::high_resolution_clock::now();
    }

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "Error al iniciar SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cout << "Error al crear ventana: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cout << "Error al crear renderer: " << SDL_GetError() << std::endl;
            return false;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, GRID_WIDTH, GRID_HEIGHT);
        if (!texture) {
            std::cout << "Error al crear textura: " << SDL_GetError() << std::endl;
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
                    grid[(y + i) % GRID_HEIGHT][(x + j) % GRID_WIDTH] = true;
                    colorGrid[(y + i) % GRID_HEIGHT][(x + j) % GRID_WIDTH] = rand() % NUM_COLORS; // Asignar color aleatorio
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
            int x = rand() % GRID_WIDTH;
            int y = rand() % GRID_HEIGHT;
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
            int x = rand() % GRID_WIDTH;
            int y = rand() % GRID_HEIGHT;
            if (!grid[y][x]) {
                grid[y][x] = true;
                colorGrid[y][x] = rand() % NUM_COLORS; // Asignar color aleatorio
                objectsPlaced++;
            }

            auto current = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = current - start;

            if (elapsed.count() > FRAME_DELAY) {
                SDL_Delay(FRAME_DELAY - elapsed.count());
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
                int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
                int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;
                count += grid[ny][nx];
            }
        }
        return count;
    }

    void update() {
        int numThreads = std::thread::hardware_concurrency();
        std::vector<std::future<void>> futures;
        int rowsPerThread = GRID_HEIGHT / numThreads;

        for (int i = 0; i < numThreads; ++i) {
            int startY = i * rowsPerThread;
            int endY = (i == numThreads - 1) ? GRID_HEIGHT : startY + rowsPerThread;

            futures.push_back(std::async(std::launch::async, [=]() {
                updateBlock(startY, endY);
            }));
        }

        for (auto& fut : futures) {
            fut.get();
        }

        std::swap(grid, nextGrid);
    }

    void render() {
        void* pixels;
        int pitch;

        if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
            std::cerr << "Error al bloquear la textura: " << SDL_GetError() << std::endl;
            return;
        }

        Uint32* pixelData = static_cast<Uint32*>(pixels);
        int numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        int rowsPerThread = GRID_HEIGHT / numThreads;

        for (int i = 0; i < numThreads; ++i) {
            int startY = i * rowsPerThread;
            int endY = (i == numThreads - 1) ? GRID_HEIGHT : startY + rowsPerThread;

            threads.push_back(std::thread([=]() {
                renderBlock(startY, endY, pixelData);
            }));
        }

        for (auto& th : threads) {
            th.join();
        }

        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void run() {
        initializeColorPalette(); // Inicializar paleta de colores
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
            int delay = FRAME_DELAY - static_cast<int>(frameDuration.count());
            if (delay > 0) {
                SDL_Delay(delay);
            }
        }
    }

    void close() {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* args[]) {
    if (argc != 2) {
        std::cout << "Uso: " << args[0] << " <número de objetos>" << std::endl;
        return 1;
    }

    int numObjects = std::atoi(args[1]);
    if (numObjects <= 0 || numObjects > GRID_WIDTH * GRID_HEIGHT) {
        std::cout << "El número de objetos debe ser positivo y no mayor que " << GRID_WIDTH * GRID_HEIGHT << std::endl;
        return 1;
    }

    Game game(numObjects);
    if (!game.init()) {
        return 1;
    }
    game.run();
    game.close();
    return 0;
}
