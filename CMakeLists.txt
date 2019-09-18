cmake_minimum_required(VERSION 3.14)
project(RayCasting)

link_directories(lib)
include_directories(include)

add_executable(RayCasting src/main.cpp)

target_link_libraries(RayCasting -lmingw32 -lSDL2main -lSDL2)