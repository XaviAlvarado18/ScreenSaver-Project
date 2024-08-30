# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = `sdl2-config --cflags --libs` -lSDL2_ttf -pthread

# Objetivo por defecto
all: conwaySeq conwayParallel conwayThreaded

# Regla para compilar conwaySeq
conwaySeq: conwaySeq.o
	$(CXX) -o conwaySeq conwaySeq.o $(LDFLAGS)

# Regla para compilar conwayParallel
conwayParallel: conwayParallel.o
	$(CXX) -o conwayParallel conwayParallel.o $(LDFLAGS)

# Regla para compilar conwayThreaded
conwayThreaded: conwayThreaded.o
	$(CXX) -o conwayThreaded conwayThreaded.o $(LDFLAGS)

# Regla para compilar archivos fuente a objetos
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f *.o conwaySeq conwayParallel conwayThreaded
