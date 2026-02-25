cmake_minimum_required(VERSION 3.10)
project(OpenGLLab1)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(lab1 main.cpp)
target_link_libraries(lab1 OpenGL::GL glfw)
