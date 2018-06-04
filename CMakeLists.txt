cmake_minimum_required(VERSION 3.10)
project(cardinal_spline)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES cardinal_spline.cpp)
set(EXECUTABLE_NAME cardinal_spline)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})