cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")

cmake_policy(SET CMP0091 NEW)

project(RayCasting)

add_executable(RayCasting src/main.cpp)

set_property(TARGET RayCasting PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(RayCasting PRIVATE cxx_std_20)
target_compile_options(RayCasting PRIVATE "$<$<CONFIG:Release>:-O2>")

find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(RayCasting PRIVATE
	SDL2::SDL2main SDL2::SDL2-static
	glm::glm)
