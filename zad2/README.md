## Building the Application

To compile on macOS with Homebrew-installed GLFW, GLAD and GLM:

```bash
g++ main.cpp src/glad.c -std=c++17 \
    -Iinclude \
    $(pkg-config --cflags --libs glfw3 glm) \
    -framework OpenGL \
    -o app;./app
