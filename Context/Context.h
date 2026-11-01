#ifndef CONTEXT_H
#define CONTEXT_H

#include "Context/Command.h"
#include "Context/Pipeline.h"
#include "Context/Shader.h"
#include "Context/Swapchain.h"
#include "Renderer/Renderer.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace mase {

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
};

} // namespace mase

#endif // CONTEXT_H
