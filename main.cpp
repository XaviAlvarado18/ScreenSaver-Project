#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    // Crear una ventana
    GLFWwindow* window = glfwCreateWindow(800, 600, "Ventana OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Hacer que la ventana sea el contexto actual
    glfwMakeContextCurrent(window);

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        // Limpiar el buffer de color
        glClear(GL_COLOR_BUFFER_BIT);

        // Aquí iría tu código de renderizado

        // Intercambiar los buffers frontal y trasero
        glfwSwapBuffers(window);

        // Procesar eventos
        glfwPollEvents();
    }

    // Limpieza
    glfwTerminate();
    return 0;
}
