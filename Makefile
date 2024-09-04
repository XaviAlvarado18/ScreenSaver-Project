# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_ttf -pthread

# Objetivo por defecto
all: ScreenSaverSeq ScreenSaverParallel ScreenSaverParallel2 ScreenSaverParallelNotC ScreenSaverThreaded

# Reglas para compilar ejecutables a partir de objetos
ScreenSaverSeq: ScreenSaverSeq.o
	$(CXX) -o ScreenSaverSeq ScreenSaverSeq.o $(LDFLAGS)

ScreenSaverParallel: ScreenSaverParallel.o
	$(CXX) -o ScreenSaverParallel ScreenSaverParallel.o $(LDFLAGS)

ScreenSaverParallel2: ScreenSaverParallel2.o
	$(CXX) -o ScreenSaverParallel2 ScreenSaverParallel2.o $(LDFLAGS)

ScreenSaverParallelNotC: ScreenSaverParallelNotC.o
	$(CXX) -o ScreenSaverParallelNotC ScreenSaverParallelNotC.o $(LDFLAGS)

ScreenSaverThreaded: ScreenSaverThreaded.o
	$(CXX) -o ScreenSaverThreaded ScreenSaverThreaded.o $(LDFLAGS)

# Regla para compilar archivos fuente a objetos
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f *.o ScreenSaverSeq ScreenSaverParallel ScreenSaverParallel2 ScreenSaverParallelNotC ScreenSaverThreaded
