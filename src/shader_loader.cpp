#include "shader_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

#include <glad/glad.h>  // Use GLAD instead of GLEW

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

// Utility to get the executable's directory
std::string getExecutableDir() {
    char path[1024];

#if defined(__APPLE__)
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
        return std::filesystem::path(path).parent_path().string();
#elif defined(_WIN32)
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path().string();
#else
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
    if (count != -1)
        return std::filesystem::path(std::string(path, count)).parent_path().string();
#endif

    return "."; // fallback
}

// Load shader source from file relative to executable
std::string loadShaderSource(const std::string& filename) {
    std::string shaderPath = getExecutableDir() + "/shaders/" + filename;
    std::ifstream shaderFile(shaderPath);

    std::cout << "Trying to load shader from: " << shaderPath << std::endl;

    if (!shaderFile.is_open()) {
        std::cerr << "Error: Could not open shader file at: " << shaderPath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    std::cout << "Shader loaded successfully from: " << shaderPath << std::endl;
    return buffer.str();
}

// Compile shader from source code
GLuint compileShader(GLenum shaderType, const std::string& source) {
    GLuint shader = glCreateShader(shaderType);
    const char* shaderSource = source.c_str();
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check for shader compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        char* log = new char[logLength];
        glGetShaderInfoLog(shader, logLength, nullptr, log);
        std::cerr << "Error compiling shader: " << log << std::endl;
        delete[] log;
    }

    return shader;
}

// Create a shader program from vertex and fragment shader files
GLuint createShaderProgramFromFile(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    std::string vertexShaderSource = loadShaderSource(vertexShaderPath);
    std::string fragmentShaderSource = loadShaderSource(fragmentShaderPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create the shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
        char* log = new char[logLength];
        glGetProgramInfoLog(shaderProgram, logLength, nullptr, log);
        std::cerr << "Error linking shader program: " << log << std::endl;
        delete[] log;
    }

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


