# Conway's Game of Life Screensaver

Este proyecto implementa un "screensaver" del famoso juego de la vida de Conway, utilizando la biblioteca SDL para la visualización gráfica y OpenMP para la paralelización del cálculo. El objetivo es explorar y aplicar técnicas de programación paralela para acelerar la ejecución del programa, comparando una versión secuencial con varias versiones paralelas.

## Descripción del Proyecto

El proyecto consiste en desarrollar un programa que dibuja en pantalla una simulación del juego de la vida de Conway. La implementación comienza con una versión secuencial, y luego se desarrolla una serie de versiones paralelas utilizando OpenMP para mejorar la eficiencia y el rendimiento del programa. El enfoque principal es aplicar paralelismo en las operaciones de actualización del estado de las células y en la generación y renderización de las figuras.

## Estructura del Proyecto

El proyecto contiene cuatro versiones del programa:

1. **Versión Secuencial (ScreenSaverSeq.cpp)**: Implementación básica del juego de la vida sin paralelismo.
2. **Versión Paralela 1 (ScreenSaverParallel.cpp)**: Añade paralelismo con OpenMP para la generación y actualización de las células.
3. **Versión Paralela 2 (ScreenSaverParallel2.cpp)**: Mejora la paralelización, incluyendo la asignación de colores aleatorios a las figuras identificadas.
4. **Versión Paralela 3 (ScreenSaverParallelNotC.cpp)**: Simplificación de la paralelización enfocándose en la actualización de las células y el renderizado, ademas de que las celulas no poseen colores.

## Requisitos

Para compilar y ejecutar este proyecto, necesitas:

- [SDL2](https://www.libsdl.org/) y [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)
- OpenMP compatible con tu compilador de C++ (por ejemplo, GCC)
- Make: Para la compilacion de los archivos.

## Compilación

Para compilar cualquiera de las versiones, usa el comando make para realizar la compilacion de todos los archivos:

```bash
make
```

## Ejecución

La ejecucion toma tres parametros:

- Cantidad de celulas a renderizar (Numero entero mayor a 0 y menor al Ancho x Alto de la pantalla).
- Ancho de la pantalla en pixeles (Debe ser un numero entero positivo).
- Alto de la pantalla en pixeles (Debe ser un numero entero positivo).

```bash
./ScreenSaverSeq <numero_de_celulas_a_renderizar> <Screen_Width> <Screen_Height>
```

```bash
./ScreenSaverParallel <numero_de_celulas_a_renderizar> <Screen_Width> <Screen_Height>
```

```bash
./ScreenSaverParallel2 <numero_de_celulas_a_renderizar> <Screen_Width> <Screen_Height>
```

```bash
./ScreenSaverParallelNotC <numero_de_celulas_a_renderizar> <Screen_Width> <Screen_Height>
```

Ejemplo:

```bash
./ScreenSaverSeq 1000 1080 720
```

## Uso de OpenMP

En las versiones paralelas, se utiliza OpenMP para paralelizar:

- La inicialización y actualización de la cuadrícula de células.
- La asignación de colores a las figuras identificadas.
- El renderizado de las células en pantalla.

La paralelización se logra utilizando directivas de OpenMP como #pragma omp parallel for para dividir el trabajo entre varios hilos y mejorar la eficiencia.

## Programas en ejecucion

![GIf](assets/ScreenSaverSeq.gif)
![GIf](assets/ScreenSaverParallel.gif)

## Resultados y Comparaciones

El objetivo es comparar la eficiencia de la versión secuencial con las versiones paralelas, evaluando la reducción en el tiempo de ejecución y el aprovechamiento de los núcleos del procesador.

## Contribuciones

Este proyecto es parte de un ejercicio para explorar técnicas de programación paralela en C++ usando OpenMP. Las contribuciones y mejoras son bienvenidas. Por favor, abre un issue o un pull request para discutir cambios o agregar mejoras.
