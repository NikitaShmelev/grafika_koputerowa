"""bash
g++ main.cpp src/glad.c -std=c++17 \
    -Iinclude \
    -I$(brew --prefix glfw)/include \
    -L$(brew --prefix glfw)/lib \
    -lglfw \
    -framework OpenGL \
    -o app
"""