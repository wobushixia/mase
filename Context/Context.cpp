#include "Context.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_vulkan.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace mase{

void Context::Init() {
  m_window = std::make_unique<mase::Window>();
  if(!m_window->GetWindow()) throw std::runtime_error("creare window failed");
  std::cout<<m_window->GetWindow();
  m_context.reset(new Context);
}

void Context::Shutdown() {
  m_context.reset();
  SDL_Quit();
}

void Context::Update() {
  while(SDL_PollEvent(&m_event)) {
    switch (m_event.type) {
      case SDL_EVENT_QUIT:
        m_windowShouldClose = true;
        break;
      default:
        break;   
    }
  }

}

Context::Context() {
  createInstance();
  pickupPhysicalDevice();
  queryQueueFamily();
  createDevice();
  getQueues();
}

void Context::createSurface() {
  VkSurfaceKHR _surface;

  std::cout << m_window->GetWindow();
  //if(!SDL_Vulkan_CreateSurface(m_window->GetWindow(), m_instance, nullptr, &_surface)) throw std::runtime_error("create SDL3 surface FAILED");
  //m_surface = vk::SurfaceKHR(_surface);
}

void Context::createInstance() {
  vk::InstanceCreateInfo ci;
  const std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

  vk::ApplicationInfo ai;
  ai.setApplicationVersion(VK_API_VERSION_1_4);
  ci.setPApplicationInfo(&ai)
    .setPEnabledLayerNames(layers);

  m_instance = vk::createInstance(ci);
}

bool isDeviceSuitable(vk::PhysicalDevice const& device) {
  auto features = device.getFeatures();

  if(features.geometryShader) return true;

  return false;
}


void Context::pickupPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices();

  for(auto device : devices) {
    if(isDeviceSuitable(device)) m_phyDevice = device;
  }
}

void Context::queryQueueFamily() {
  auto queueFamilies = m_phyDevice.getQueueFamilyProperties();

  for(size_t i = 0; i < queueFamilies.size(); i++) {
    const auto& queueFamily = queueFamilies[i];

    if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      m_queueFamilyIndices.graphicsFamily = i;
      break;
    }
  }
}

void Context::createDevice() {
  vk::DeviceCreateInfo deviceCI;
  vk::DeviceQueueCreateInfo queueCI;

  float priorities = 1.0f;

  queueCI.setPQueuePriorities(&priorities)
         .setQueueCount(1)
         .setQueueFamilyIndex(m_queueFamilyIndices.graphicsFamily.value());
  deviceCI.setQueueCreateInfos(queueCI);

  m_device = m_phyDevice.createDevice(deviceCI);
}

void Context::getQueues() {
  m_graphicsQueue = m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value(), 0);
}

Context::~Context() {
  m_device.destroy();
  m_instance.destroy();
}

Context& Context::GetInstance() {
  return *m_context;
}

Window& Context::GetWindow() {
  return *m_window;
}

}
