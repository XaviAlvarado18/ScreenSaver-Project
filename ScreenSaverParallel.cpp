/*
    Conway's Game of Life Parallel Version
    =======================================
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
#include <omp.h>  // Incluir la biblioteca de OpenMP
#include <unordered_map>
#include <queue>

struct Color {
    Uint8 r, g, b, a;
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;
    std::vector<std::vector<int>> figureIds; // Identificador de figuras
    std::unordered_map<int, Color> figureColors;
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
    Game(int objects, int width, int height, int threads, int cell_size = 10) 
        : window(nullptr), renderer(nullptr), texture(nullptr), frameCount(0), fps(0),
          numObjects(objects), screenWidth(width), screenHeight(height), cellSize(cell_size), numThreads(threads) {
        gridWidth = screenWidth / cellSize;
        gridHeight = screenHeight / cellSize;

        grid.resize(gridHeight, std::vector<bool>(gridWidth, false));
        nextGrid.resize(gridHeight, std::vector<bool>(gridWidth, false));
        figureIds.resize(gridHeight, std::vector<int>(gridWidth, -1));
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

    void randomizeGrid() {
        srand(time(nullptr));
        int objectsPlaced = 0;  // Contador para rastrear el número de células activadas

        #pragma omp parallel for collapse(2) num_threads(numThreads) schedule(static)
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                if (objectsPlaced < numObjects) {  // Verifica si aún necesitas más objetos
                    bool shouldPlace = rand() % 2 == 0;

                    if (shouldPlace) {
                        #pragma omp atomic
                        objectsPlaced++;  // Incrementar de manera atómica

                        // Solo activa la célula si no se ha superado el límite de objetos
                        if (objectsPlaced <= numObjects) {
                            grid[y][x] = true;
                        }
                    }
                }
            }
        }

        assignFigureColors();
    }

    void assignFigureColors() {
        int figureId = 0;
        figureColors.clear();

        #pragma omp parallel for num_threads(numThreads) schedule(static)
        for (int y = 0; y < gridHeight; ++y) {
            std::fill(figureIds[y].begin(), figureIds[y].end(), -1);
        }

        #pragma omp parallel for num_threads(numThreads) schedule(static)
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                if (grid[y][x] && figureIds[y][x] == -1) {
                    assignFigureId(x, y, figureId);

                    #pragma omp critical
                    {
                        figureColors[figureId] = generateRandomColor();
                        figureId++;
                    }
                }
            }
        }
    }

    void assignFigureId(int startX, int startY, int figureId) {
        std::queue<std::pair<int, int>> toVisit;
        toVisit.push({startX, startY});

        while (!toVisit.empty()) {
            auto [x, y] = toVisit.front();
            toVisit.pop();

            if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) continue;
            if (!grid[y][x] || figureIds[y][x] != -1) continue;

            figureIds[y][x] = figureId;

            toVisit.push({x - 1, y});
            toVisit.push({x + 1, y});
            toVisit.push({x, y - 1});
            toVisit.push({x, y + 1});
        }
    }

    Color generateRandomColor() {
        return { static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), 255 };
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
        #pragma omp parallel for collapse(2) num_threads(numThreads) schedule(dynamic) 
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

        #pragma omp parallel for collapse(2) num_threads(numThreads) reduction(+:frameCount)  
        for (int y = 0; y < gridHeight; ++y) {
            for (int x = 0; x < gridWidth; ++x) {
                if (grid[y][x]) {
                    int figureId = figureIds[y][x];
                    Color color = figureColors[figureId];
                    pixelData[y * gridWidth + x] = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), color.r, color.g, color.b, color.a);
                } else {
                    pixelData[y * gridWidth + x] = 0x000000FF;
                }
            }
        }

        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void run() {
        randomizeGrid();

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }

            update();
            render();
            calculateFPS();
            SDL_Delay(16);  // Limita a aproximadamente 60 FPS
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
        std::cerr << "Uso: " << args[0] << " <número de células> <ancho> <alto> <número de hilos>" << std::endl;
        return 1;
    }

    int numObjects = std::atoi(args[1]);
    int screenWidth = std::atoi(args[2]);
    int screenHeight = std::atoi(args[3]);
    int numThreads = std::atoi(args[4]);

    if (numObjects <= 0 || screenWidth <= 0 || screenHeight <= 0 || numThreads <= 0) {
        std::cerr << "Todos los parámetros deben ser positivos y mayores que cero." << std::endl;
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
