#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int CELL_SIZE = 10;
const int GRID_WIDTH = SCREEN_WIDTH / CELL_SIZE;
const int GRID_HEIGHT = SCREEN_HEIGHT / CELL_SIZE;

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

public:
    Game(int objects) : window(nullptr), renderer(nullptr), texture(nullptr), frameCount(0), fps(0), numObjects(objects) {
        grid.resize(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
        nextGrid.resize(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
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

    void randomizeGrid() {
        srand(time(nullptr));
        int objectsPlaced = 0;
        int centerX = GRID_WIDTH / 2;
        int centerY = GRID_HEIGHT / 2;
        int range = std::min(GRID_WIDTH, GRID_HEIGHT) / 4; // Rango para dispersión cerca del centro

        while (objectsPlaced < numObjects) {
            int x = centerX + (rand() % (2 * range)) - range; // Centrado alrededor del centro
            int y = centerY + (rand() % (2 * range)) - range;
            
            // Asegura que x y y estén dentro de los límites
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT && !grid[y][x]) {
                grid[y][x] = true;
                objectsPlaced++;
            }
        }

        std::cout << "Se acabo "<<std::endl;
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
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
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

        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                pixelData[y * GRID_WIDTH + x] = grid[y][x] ? 0xFFFFFFFF : 0x000000FF;
            }
        }

        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void run() {
        randomizeGrid();

        // Medir tiempo de ejecución
        auto start = std::chrono::high_resolution_clock::now();

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
            SDL_Delay(16);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Tiempo de ejecución secuencial: " << duration.count() << " segundos" << std::endl;
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
