cmake_minimum_required(VERSION 3.20)
project(mase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)

find_program(SLANG_PROGRAM slang REQUIRED)
execute_process(COMMAND ${SLANG_PROGRAM} ${CMAKE_SOURCE_DIR}/Assets/shaders/shader.slang -o ${CMAKE_SOURCE_DIR}/Assets/shaders/shader.spv)

add_library(mase_core STATIC
  Context/Context.cpp
  Context/Swapchain.cpp
  Renderer/Renderer.cpp
  Window/Window.cpp
  Mase/Mase.cpp
)

target_include_directories(mase_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mase_core PUBLIC SDL3::SDL3 Vulkan::Vulkan)

add_executable(mase main.cpp)
target_link_libraries(mase PRIVATE mase_core)
target_compile_definitions(mase PRIVATE SDL_MAIN_HANDLED)
