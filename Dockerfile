# Usa una imagen base de Ubuntu
FROM ubuntu:20.04

# Instala herramientas necesarias
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libsdl2-dev \
    && rm -rf /var/lib/apt/lists/*

# Crea un directorio para el proyecto
WORKDIR /app

# Copia el código fuente al contenedor
COPY . /app

# Compila el código
RUN g++ -o myapp main.cpp `sdl2-config --cflags --libs`

# Ejecuta el ejecutable
CMD ["./myapp"]
