#ifndef CONTEXT_H
#define CONTEXT_H

#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
};

namespace mase {

class Context {
public:
  Context();
  ~Context();

  Context& GetInstance();
  Window& GetWindow();
  void Init();
  void Update();
  void Shutdown();
  bool GetWindowShouldClose() {return m_windowShouldClose;}

private:
  std::unique_ptr<mase::Context> m_context = nullptr;
  std::unique_ptr<mase::Window> m_window = nullptr;
  vk::SurfaceKHR m_surface;
  SDL_Event m_event;
  bool m_windowShouldClose;

  vk::Instance m_instance;
  vk::PhysicalDevice m_phyDevice;
  QueueFamilyIndices m_queueFamilyIndices;
  vk::Device m_device;
  vk::Queue m_graphicsQueue;

  void createInstance();
  void pickupPhysicalDevice();
  void createDevice();
  void queryQueueFamily();
  void getQueues();
  void createSurface();
};

}

#endif //CONTEXT_H
