cmake_minimum_required(VERSION 3.12)
project(2d-renderer VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

# compile shaders
execute_process(COMMAND python3 shader_gen.py)

set(INCLUDE src thirdparty)
file(GLOB_RECURSE SOURCE src/*.cpp)

add_executable(app ${SOURCE}
	thirdparty/stb/stb_image.cpp
	thirdparty/vma/vk_mem_alloc.cpp
)

target_include_directories(app PRIVATE ${INCLUDE})
target_compile_options(app PRIVATE -Wall)
target_link_libraries(app PRIVATE Vulkan::Vulkan SDL2::SDL2)
