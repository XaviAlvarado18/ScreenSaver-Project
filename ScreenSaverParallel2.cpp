/*
    Conway's Game of Life Parallel Version - Parallel2
    ==================================================
    Este programa implementa una versión paralela del "Conway's Game of Life" utilizando
    C++, la librería SDL para visualización gráfica, y OpenMP para paralelización.
    
    Características:
    - Paralelización con OpenMP para mejorar el rendimiento.
    - Visualización de figuras con colores aleatorios.
    - Recibe parámetros de entrada para ajustar el número de células, ancho, alto y número de hilos.

    Autores: [Kristopher Alvarado, David Aragon y Renatto Guzman]
    Fecha: 05/09/2024
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <string>
#include <omp.h> // Incluir OpenMP

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;
    std::vector<std::vector<Uint32>> colorGrid; // Matriz para almacenar los colores de las figuras
    int frameCount;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    float fps;
    int screenWidth;
    int screenHeight;
    int cellSize;
    int gridWidth;
    int gridHeight;
    int numObjects;
    int numThreads; // Número de hilos

public:
    Game(int objects, int width, int height, int threads, int cell_size = 12)
        : window(nullptr), renderer(nullptr), texture(nullptr), frameCount(0), fps(0),
          numObjects(objects), screenWidth(width), screenHeight(height), cellSize(cell_size), numThreads(threads) {
        gridWidth = screenWidth / cellSize;
        gridHeight = screenHeight / cellSize;

        grid.resize(gridHeight, std::vector<bool>(gridWidth, false));
        nextGrid.resize(gridHeight, std::vector<bool>(gridWidth, false));
        colorGrid.resize(gridHeight, std::vector<Uint32>(gridWidth, 0x000000FF)); // Inicializar color de fondo
        lastTime = std::chrono::high_resolution_clock::now();
    }

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "Error al iniciar SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                  screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cout << "Error al crear ventana: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cout << "Error al crear renderer: " << SDL_GetError() << std::endl;
            return false;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gridWidth, gridHeight);
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
        #pragma omp single  // Solo un hilo realiza esta operación
        {
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
    }

    void placePattern(int x, int y, const std::vector<std::vector<int>>& pattern, Uint32 color) {
        int patternHeight = pattern.size();
        int patternWidth = pattern[0].size(); 

        #pragma omp parallel for num_threads(numThreads) collapse(2)
        for (int i = 0; i < patternHeight; ++i) {
            for (int j = 0; j < patternWidth; ++j) {
                if (pattern[i][j] == 1) {
                    int posX = (x + j) % gridWidth;
                    int posY = (y + i) % gridHeight;
                    grid[posY][posX] = true;
                    colorGrid[posY][posX] = color; 
                }
            }
        }
    }

    Uint32 generateColorFromIndex(int index) {
        Uint8 r = (index * 123 + 45) % 256;
        Uint8 g = (index * 67 + 89) % 256;
        Uint8 b = (index * 89 + 123) % 256;
        return (r << 24) | (g << 16) | (b << 8) | 0xFF; // Color RGBA
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

        #pragma omp parallel for num_threads(numThreads)
        for (int i = 0; i < numObjects; ++i) {
            int patternIndex = rand() % patterns.size();
            int x = rand() % gridWidth;
            int y = rand() % gridHeight;
            Uint32 color = generateColorFromIndex(patternIndex); 
            placePattern(x, y, patterns[patternIndex], color); 
        }

        auto end = std::chrono::high_resolution_clock::now(); // Fin de medición de tiempo
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tiempo para generar figuras: " << duration.count() << " segundos" << std::endl;
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
        #pragma omp parallel for collapse(2) num_threads(numThreads)
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                int neighbors = countNeighbors(x, y);
                nextGrid[y][x] = (grid[y][x] && (neighbors == 2 || neighbors == 3)) || (!grid[y][x] && neighbors == 3);
            }
        }
        std::swap(grid, nextGrid);
    }

    void render() {
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        Uint32* pixelData = static_cast<Uint32*>(pixels);

        #pragma omp parallel for collapse(2) num_threads(numThreads)
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                pixelData[y * gridWidth + x] = grid[y][x] ? colorGrid[y][x] : 0x000000FF; 
            }
        }

        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void run() {
        generateFigures(); 

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
            int delay = (1000 / 60) - static_cast<int>(frameDuration.count());
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
        SDL_Quit();
    }
};

int main(int argc, char* args[]) {
    if (argc != 5) {
        std::cout << "Uso: " << args[0] << " <número de objetos> <ancho> <alto> <número de hilos>" << std::endl;
        return 1;
    }

    int numObjects = std::atoi(args[1]);
    int screenWidth = std::atoi(args[2]);
    int screenHeight = std::atoi(args[3]);
    int numThreads = std::atoi(args[4]);

    if (numObjects <= 0 || screenWidth <= 0 || screenHeight <= 0 || numThreads <= 0) {
        std::cout << "Todos los parámetros deben ser positivos y mayores que cero." << std::endl;
        return 1;
    }

    Game game(numObjects, screenWidth, screenHeight, numThreads);
    if (!game.init()) {
        game.close();
        return 1;
    }
    game.run();
    game.close();
    return 0;
}
