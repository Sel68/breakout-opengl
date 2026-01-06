#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void inputProcess(GLFWwindow *window);

std::string LoadShaderSource (const char* filepath){
    std::ifstream file(filepath); std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(640, 480, "WINDOW NAME", NULL, NULL);
    if (!window){
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate(); return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to init GLAD\n"; return -1;
    }

    std::string vertexCode = LoadShaderSource("shader.vert");
    const char* vShaderCode = vertexCode.c_str();

    std::string fragCode = LoadShaderSource("shader.frag");
    const char* fShaderCode = fragCode.c_str();

    while(!glfwWindowShouldClose(window)){
        inputProcess(window);
        
        glfwSwapBuffers(window); glfwPollEvents();
    }
    return 0;
}

void inputProcess(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}