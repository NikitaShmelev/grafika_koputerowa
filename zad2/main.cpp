#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Program wyświetla 2 trójkąty i 2 prostokąty.
// Każda figura ma inny kolor i jest animowana.
// Figury nie wychodzą poza obszar okna dzięki detekcji kolizji i odbiciom (dla Figury 1 i 4).

// Wymiary okna
const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

// Vertex shader
const char* vertexShaderSrc = R"(#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uTransform;  // Transformacja modeli
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
}
)";

// Fragment shader
const char* fragmentShaderSrc = R"(#version 330 core
out vec4 FragColor;
uniform vec4 uColor;  // Kolor figury
void main() { FragColor = uColor; }
)";

// Kompilacja shadera i sprawdzanie błędów
unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetShaderInfoLog(shader, 512, nullptr, buf);
        std::cerr << "Shader compile error: " << buf << std::endl;
    }
    return shader;
}

int main() {
    // Inicjalizacja GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Animated Shapes", nullptr, nullptr);
    if (!window) {
        std::cerr << "Nie można utworzyć okna" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Wczytanie funkcji OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Nie udało się zainicjalizować GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Budowa programu shaderów
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Dane geometrii: trójkąt i prostokąt
    float triVertices[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };
    float rectVertices[] = {
       -0.5f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f,  0.5f, 0.0f
    };
    unsigned int rectIndices[] = { 0,1,2, 0,2,3 };

    // VAO/VBO dla trójkąta
    unsigned int VAO_tri, VBO_tri;
    glGenVertexArrays(1, &VAO_tri);
    glGenBuffers(1, &VBO_tri);
    glBindVertexArray(VAO_tri);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tri);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    // VAO/VBO/EBO dla prostokąta
    unsigned int VAO_rect, VBO_rect, EBO_rect;
    glGenVertexArrays(1,&VAO_rect);
    glGenBuffers(1,&VBO_rect);
    glGenBuffers(1,&EBO_rect);
    glBindVertexArray(VAO_rect);
    glBindBuffer(GL_ARRAY_BUFFER,VBO_rect);
    glBufferData(GL_ARRAY_BUFFER,sizeof(rectVertices),rectVertices,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO_rect);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(rectIndices),rectIndices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    // Lokalizacje uniformów
    glUseProgram(prog);
    int locTrans = glGetUniformLocation(prog, "uTransform");
    int locColor = glGetUniformLocation(prog, "uColor");

    // Parametry animacji i początkowe pozycje
    const float halfTri = 0.5f * 0.6f; // pół rozmiaru trójkąta w skali
    const float halfRec = 0.5f * (0.4f + 0.1f);
    
    // --- Ruch Figury 1: trójkąt z odbiciami ---
    static glm::vec2 pos1(-0.6f, 0.6f);
    static glm::vec2 vel1 = glm::normalize(glm::vec2(1.0f,0.5f)) * 0.5f;

    // --- Ruch Figury 4: prostokąt z łączoną animacją ---
    static glm::vec2 pos4( 0.6f,-0.6f);
    static glm::vec2 vel4 = glm::normalize(glm::vec2(-1.0f,0.3f)) * 0.4f;

    // Pętla główna
    while (!glfwWindowShouldClose(window)) {
        float t = (float)glfwGetTime();
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);

        // --- Figura 1 ---
        // Trójkąt: ruch prostoliniowy z odbiciami od krawędzi okna
        {
            float dt = 0.01f;
            pos1 += vel1 * dt;
            // Odbicie w poziomie
            if (pos1.x + halfTri > 1.0f || pos1.x - halfTri < -1.0f) vel1.x *= -1;
            // Odbicie w pionie
            if (pos1.y + halfTri > 1.0f || pos1.y - halfTri < -1.0f) vel1.y *= -1;
            glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(pos1,0.0f));
            M = glm::scale(M, glm::vec3(0.6f));
            glUniformMatrix4fv(locTrans,1,GL_FALSE,glm::value_ptr(M));
            glUniform4f(locColor,1.0f,0.0f,0.0f,1.0f);
            glBindVertexArray(VAO_tri);
            glDrawArrays(GL_TRIANGLES,0,3);
        }

        // --- Figura 2 ---
        // Trójkąt: stała rotacja w miejscu (górny prawy)
        {
            glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(0.6f,0.6f,0.0f));
            M = glm::rotate(M, t, glm::vec3(0,0,1));
            M = glm::scale(M, glm::vec3(0.6f));
            glUniformMatrix4fv(locTrans,1,GL_FALSE,glm::value_ptr(M));
            glUniform4f(locColor,0.0f,1.0f,0.0f,1.0f);
            glBindVertexArray(VAO_tri);
            glDrawArrays(GL_TRIANGLES,0,3);
        }

        // --- Figura 3 ---
        // Prostokąt: pulsacyjne powiększanie i zmniejszanie (dolny lewy)
        {
            float s = 0.4f + sin(t*2.0f)*0.1f;
            glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(-0.6f,-0.6f,0.0f));
            M = glm::scale(M, glm::vec3(s));
            glUniformMatrix4fv(locTrans,1,GL_FALSE,glm::value_ptr(M));
            glUniform4f(locColor,0.0f,0.0f,1.0f,1.0f);
            glBindVertexArray(VAO_rect);
            glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
        }

        // --- Figura 4 ---
        // Prostokąt: łączona animacja ruchu z odbiciami, rotacji i skali
        {
            float dt = 0.01f;
            pos4 += vel4 * dt;
            if (pos4.x + halfRec > 1.0f || pos4.x - halfRec < -1.0f) vel4.x *= -1;
            if (pos4.y + halfRec > 1.0f || pos4.y - halfRec < -1.0f) vel4.y *= -1;
            float sc = 0.4f + cos(t)*0.1f;
            glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(pos4,0.0f));
            M = glm::rotate(M, t, glm::vec3(0,0,1));
            M = glm::scale(M, glm::vec3(sc));
            glUniformMatrix4fv(locTrans,1,GL_FALSE,glm::value_ptr(M));
            glUniform4f(locColor,1.0f,1.0f,0.0f,1.0f);
            glBindVertexArray(VAO_rect);
            glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
        }

        // Zamiana buforów i przetwarzanie zdarzeń
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Sprzątanie
    glDeleteVertexArrays(1,&VAO_tri);
    glDeleteBuffers(1,&VBO_tri);
    glDeleteVertexArrays(1,&VAO_rect);
    glDeleteBuffers(1,&VBO_rect);
    glDeleteBuffers(1,&EBO_rect);
    glDeleteProgram(prog);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
