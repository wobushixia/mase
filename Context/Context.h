#ifndef CONTEXT_H
#define CONTEXT_H

#include "Context/Command.h"
#include "Context/Pipeline.h"
#include "Context/Shader.h"
#include "Context/Swapchain.h"
#include "Renderer/Renderer.h"
#include "UI/Ui.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace mase {

struct GpuVertex {
  VkBuffer buffer;
  VmaAllocation allocation;
  VkDeviceSize vertexOffset;
  VkDeviceSize indexOffset;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;
};

struct Image {
  VkFormat format;
  VkImage img;
  VkImageView imgView;
  VmaAllocation allocation;
};

struct ShaderData {
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
  glm::vec4 lightPos{0.0f, -10.0f, 10.0f, 0.0f};
};

struct ShaderDataBuffer {
  VmaAllocation alloc{VK_NULL_HANDLE};
  VmaAllocationInfo allocInfo{};
  VkBuffer buf{VK_NULL_HANDLE};
  VkDeviceAddress deviceAddr{};
};

class Context {
public:
  Context();
  ~Context();

  static Context &GetInstance();

  void Init();
  void Update();
  void Shutdown();
  bool GetWindowShouldClose() { return m_windowShouldClose; }
  Window &GetWindow();
  vk::SurfaceKHR GetSurface() { return m_surface; }

  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  Window m_window;
  vk::SurfaceKHR m_surface;
  SDL_Event m_event;
  bool m_windowShouldClose;

  const int MAX_FRAMES_IN_FLIGHT = 2;

  vk::Instance m_instance;
  vk::PhysicalDevice m_phyDevice;
  uint32_t m_qfpIndex = 0;
  vk::Device m_device;
  vk::Queue m_queue;
  Swapchain m_swapchain;
  Shader m_shader;
  Pipeline m_pipeline;
  CommandManager m_cmdMan;
  Renderer m_renderer;
  VmaAllocator m_allocator;
  Image m_depth;
  GpuVertex vBuffer;
  VkDeviceSize m_indexCount;
  std::vector<ShaderDataBuffer> m_SDBufs;
  UiManager m_uiMan;

private:
  void createWindow();
  void createInstance();
  void pickupPhysicalDevice();
  void createDevice();
  void queryQueueFamily();
  void getQueues();
  void createSurface();
  void createSwapchain();
  void createShaders();
  void createPipeline();
  void createCommandManager();
  void createRenderer();
  void initVma();
  void createDepthBuffer();
  void loadModel();
  void createSDBuffers();
  void updateSDBuffers();
  void createUiManager();

  void RenderUi();
};

} // namespace mase

#endif // CONTEXT_H
