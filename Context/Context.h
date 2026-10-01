#ifndef CONTEXT_H
#define CONTEXT_H

#include "Context/Swapchain.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

namespace mase {

class Context {
public:
  Context();
  ~Context();

  static Context& GetInstance();

  void Init();
  void Update();
  void Shutdown();
  bool GetWindowShouldClose() { return m_windowShouldClose; }
  Window& GetWindow();
  vk::SurfaceKHR GetSurface() { return m_surface; }

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  std::unique_ptr<mase::Window> m_window = nullptr;
  vk::SurfaceKHR m_surface;
  SDL_Event m_event;
  bool m_windowShouldClose;

  vk::Instance m_instance;
  vk::PhysicalDevice m_phyDevice;
  QueueFamilyIndices m_queueFamilyIndices;
  vk::Device m_device;
  vk::Queue m_graphicsQueue;
  vk::Queue m_presentQueue;
  std::unique_ptr<Swapchain> m_swapchain;

private:
  void createInstance();
  void pickupPhysicalDevice();
  void createDevice();
  void queryQueueFamily();
  void getQueues();
  void createSurface();
};

}

#endif //CONTEXT_H
