cmake_minimum_required(VERSION 3.20)
project(mase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glm CONFIG REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_program(SLANG_PROGRAM slang REQUIRED)

set(SHADER_VERT_SPV ${CMAKE_CURRENT_SOURCE_DIR}/Assets/shaders/shader.vert.spv)
set(SHADER_FRAG_SPV ${CMAKE_CURRENT_SOURCE_DIR}/Assets/shaders/shader.frag.spv)

add_custom_command(
  OUTPUT ${SHADER_VERT_SPV} ${SHADER_FRAG_SPV}
  COMMAND ${SLANG_PROGRAM} ${SHADER_SRC}
          -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name
          -entry vertMain -stage vertex   -o ${SHADER_VERT_SPV}
          -entry fragMain -stage fragment -o ${SHADER_FRAG_SPV}
  DEPENDS ${SHADER_SRC}
  COMMENT "Compiling shader.slang -> vert/frag SPIR-V"
  VERBATIM
)

add_custom_target(shaders ALL DEPENDS ${SHADER_SPV})

file(GLOB IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_sdl3.cpp
)

file(GLOB_RECURSE MASE_CORE_SOURCES CONFIGURE_DEPENDS
  Context/*.cpp
  Base/*.cpp
  Renderer/*.cpp
  Window/*.cpp
  Mase/*.cpp
  UI/*.cpp
)

add_library(mase_core STATIC ${MASE_CORE_SOURCES} ${IMGUI_SRC})

target_include_directories(mase_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends
)

target_link_libraries(mase_core PUBLIC glm::glm SDL3::SDL3 Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)

add_executable(mase main.cpp)
target_link_libraries(mase PRIVATE mase_core)
target_compile_definitions(mase PRIVATE SDL_MAIN_HANDLED)
add_dependencies(mase shaders)
