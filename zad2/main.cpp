#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Vertex shader
const char* vertexShaderSrc = R"(#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uTransform;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
}
)";

// Fragment shader
const char* fragmentShaderSrc = R"(#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main() {
    FragColor = uColor;
}
)";

// Window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Utility: compile shader and check errors
unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    int success;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return sh;
}

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Animated Shapes", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Build shader program
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    unsigned int shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vs); glAttachShader(shaderProg, fs);
    glLinkProgram(shaderProg);
    glDeleteShader(vs); glDeleteShader(fs);

    // Define geometry
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

    // VAO/VBO for triangle
    unsigned int VAO_tri, VBO_tri;
    glGenVertexArrays(1, &VAO_tri);
    glGenBuffers(1, &VBO_tri);
    glBindVertexArray(VAO_tri);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_tri);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // VAO/VBO/EBO for rectangle
    unsigned int VAO_rect, VBO_rect, EBO_rect;
    glGenVertexArrays(1, &VAO_rect);
    glGenBuffers(1, &VBO_rect);
    glGenBuffers(1, &EBO_rect);
    glBindVertexArray(VAO_rect);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_rect);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_rect);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndices), rectIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    // Get uniform locations
    glUseProgram(shaderProg);
    int uniTrans = glGetUniformLocation(shaderProg, "uTransform");
    int uniColor = glGetUniformLocation(shaderProg, "uColor");

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float t = glfwGetTime();
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProg);

        // Shape1: triangle, straight line (horizontal sine)
        glm::mat4 trans1 = glm::translate(glm::mat4(1.0f), glm::vec3(sin(t)*0.5f, 0.6f, 0.0f));
        glUniformMatrix4fv(uniTrans,1,GL_FALSE,glm::value_ptr(trans1));
        glUniform4f(uniColor,1.0f,0.0f,0.0f,1.0f);
        glBindVertexArray(VAO_tri);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Shape2: triangle, rotation
        glm::mat4 trans2 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.6f, -0.6f,0.0f));
        trans2 = glm::rotate(trans2, t, glm::vec3(0.0f,0.0f,1.0f));
        glUniformMatrix4fv(uniTrans,1,GL_FALSE,glm::value_ptr(trans2));
        glUniform4f(uniColor,0.0f,1.0f,0.0f,1.0f);
        glBindVertexArray(VAO_tri);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Shape3: rectangle, scale pulsate
        float scale = (sin(t*2.0f)*0.5f + 1.0f) * 0.25f + 0.25f;
        glm::mat4 trans3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f,0.0f));
        trans3 = glm::scale(trans3, glm::vec3(scale));
        glUniformMatrix4fv(uniTrans,1,GL_FALSE,glm::value_ptr(trans3));
        glUniform4f(uniColor,0.0f,0.0f,1.0f,1.0f);
        glBindVertexArray(VAO_rect);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        // Shape4: rectangle, combine (translate+rotate+scale)
        glm::mat4 trans4 = glm::translate(glm::mat4(1.0f), glm::vec3(sin(t)*0.4f, cos(t)*0.4f,0.0f));
        trans4 = glm::rotate(trans4, t, glm::vec3(0.0f,0.0f,1.0f));
        float scl = (cos(t)*0.5f + 1.0f) * 0.25f + 0.25f;
        trans4 = glm::scale(trans4, glm::vec3(scl));
        glUniformMatrix4fv(uniTrans,1,GL_FALSE,glm::value_ptr(trans4));
        glUniform4f(uniColor,1.0f,1.0f,0.0f,1.0f);
        glBindVertexArray(VAO_rect);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        // Swap
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1,&VAO_tri);
    glDeleteBuffers(1,&VBO_tri);
    glDeleteVertexArrays(1,&VAO_rect);
    glDeleteBuffers(1,&VBO_rect);
    glDeleteBuffers(1,&EBO_rect);
    glDeleteProgram(shaderProg);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
