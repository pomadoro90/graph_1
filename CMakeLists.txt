cmake_minimum_required(VERSION 3.10)
project(OpenGLLab1)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(lab1 main.cpp)
target_link_libraries(lab1 OpenGL::GL OpenGL::GLU GLUT::GLUT)
