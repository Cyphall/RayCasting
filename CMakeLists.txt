cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

cmake_policy(SET CMP0091 NEW)

project(RayCasting)

add_compile_options(/Zc:__cplusplus)
add_compile_options(/utf-8)

add_executable(RayCasting
		src/main.cpp
)

set_property(TARGET RayCasting PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(RayCasting PRIVATE cxx_std_20)

target_include_directories(RayCasting PRIVATE "src")

find_package(glm CONFIG REQUIRED)
target_link_libraries(RayCasting PRIVATE glm::glm)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(RayCasting PRIVATE SDL2::SDL2main SDL2::SDL2-static)