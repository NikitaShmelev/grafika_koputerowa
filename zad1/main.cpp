#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Vertex shader source
const char* vertexShaderSource = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
out vec2 TexCoords;
void main()
{
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoords;
})";

// Fragment shader source
const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D uTexture;
uniform vec4 uColor;
uniform float uMixFactor;
void main()
{
    vec4 texColor = texture(uTexture, TexCoords);
    FragColor = mix(texColor, uColor, uMixFactor);
}
)";

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Globals for mixing
float mixFactor = 0.0f;
int mixLoc, colorLoc, texLoc;
unsigned int shaderProgram;

// Scroll callback: adjust mix factor
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    mixFactor += static_cast<float>(yoffset) * 0.05f;
    mixFactor = std::clamp(mixFactor, 0.0f, 1.0f);
    glUseProgram(shaderProgram);
    glUniform1f(mixLoc, mixFactor);
}

bool showSquare   = false;
bool showTriangle = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // reagujemy tylko na pojedyncze naciśnięcie
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_Q) {
            showSquare = !showSquare; // toggle kwadratu
        }
        else if (key == GLFW_KEY_T) {
            showTriangle = !showTriangle; // toggle trójkąta
        }
        else if (key == GLFW_KEY_B) {
            // 2 na raz
            bool both = showSquare && showTriangle;
            showSquare = showTriangle = !both;
        }
    }
}

int main()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shapes with Textures", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL functions with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback   (window, key_callback);

    // Build and compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // Check compile errors...
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Geometry: square (left) and triangle (right)
    float squareVertices[] = {
        // pos             // tex
        -0.9f,  0.75f, 0.0f,  0.0f, 1.0f,
        -0.9f, -0.75f, 0.0f,  0.0f, 0.0f,
        -0.1f, -0.75f, 0.0f,  1.0f, 0.0f,
        -0.9f,  0.75f, 0.0f,  0.0f, 1.0f,
        -0.1f, -0.75f, 0.0f,  1.0f, 0.0f,
        -0.1f,  0.75f, 0.0f,  1.0f, 1.0f
    };
    float triangleVertices[] = {
        // pos             // tex
         0.1f, -0.75f, 0.0f,  0.0f, 0.0f,
         0.9f, -0.75f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.75f, 0.0f,  0.5f, 1.0f
    };

    unsigned int VAO1, VBO1, VAO2, VBO2;
    // Square setup
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Triangle setup
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load textures (place texture1.jpg and texture2.jpg in working dir)
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("texture1.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else std::cerr << "Failed to load texture1.jpg\n";
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load("texture2.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else std::cerr << "Failed to load texture2.jpg\n";
    stbi_image_free(data);

    // Configure shader uniforms
    glUseProgram(shaderProgram);
    texLoc   = glGetUniformLocation(shaderProgram, "uTexture");
    colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    mixLoc   = glGetUniformLocation(shaderProgram, "uMixFactor");
    glUniform1i(texLoc, 0);
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glUniform1f(mixLoc, mixFactor);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        if (showSquare) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glBindVertexArray(VAO1);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        if (showTriangle) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture2);
            glBindVertexArray(VAO2);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
