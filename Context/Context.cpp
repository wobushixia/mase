#include "Context.h"
#include "Context/Swapchain.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <ostream>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace mase {

Context::Context() {}

void Context::Shutdown() {
  m_device.waitIdle();
  m_swapchain.reset();
  m_device.destroy();
  m_instance.destroySurfaceKHR(m_surface);
  m_instance.destroy();
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

void Context::Init() {
  m_window = std::make_unique<mase::Window>();
  createInstance();
  createSurface();
  pickupPhysicalDevice();
  queryQueueFamily();
  createDevice();
  getQueues();
  m_swapchain = std::make_unique<Swapchain>();
}

Context& Context::GetInstance() {
    static Context instance;
    return instance;
}

void Context::createSurface() {
  VkSurfaceKHR _surface;

  if(!SDL_Vulkan_CreateSurface(m_window->GetWindow(), m_instance, nullptr, &_surface)) throw std::runtime_error("create SDL3 surface FAILED");
  m_surface = vk::SurfaceKHR(_surface);
}

void Context::createInstance() {
  vk::InstanceCreateInfo ci;
  const std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

  Uint32 sdlExtCount = 0;
  const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
  std::vector extensions(sdlExts, sdlExts + sdlExtCount);
  
  vk::ApplicationInfo ai;
  ai.setApiVersion(VK_API_VERSION_1_4);
  ci.setPApplicationInfo(&ai)
    .setPEnabledLayerNames(layers)
    .setPEnabledExtensionNames(extensions);

  m_instance = vk::createInstance(ci);
}

bool isDeviceSuitable(vk::PhysicalDevice const& device) {
  auto features = device.getFeatures();

  if(features.geometryShader) return true;

  return false;
}


void Context::pickupPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices();
  m_phyDevice = devices[0];

  for(auto device : devices) {
    if(isDeviceSuitable(device)) m_phyDevice = device;
    break;
  }
}

void Context::queryQueueFamily() {
  auto queueFamilies = m_phyDevice.getQueueFamilyProperties();

  for(size_t i = 0; i < queueFamilies.size(); i++) {
    const auto& queueFamily = queueFamilies[i];

    if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      m_queueFamilyIndices.graphicsFamily = i;
    }
    if(m_phyDevice.getSurfaceSupportKHR(i, m_surface)) {
      m_queueFamilyIndices.presentFamily = i;
    }

    if(m_queueFamilyIndices.isComplete()) break;
  }
}

void Context::createDevice() {
  vk::DeviceCreateInfo deviceCI;
  std::vector<vk::DeviceQueueCreateInfo> queueCIs;

  float priorities = 1.0f;

  if(m_queueFamilyIndices.presentFamily == m_queueFamilyIndices.graphicsFamily) {
    vk::DeviceQueueCreateInfo queueCI;
    queueCI.setPQueuePriorities(&priorities)
           .setQueueCount(1)
           .setQueueFamilyIndex(m_queueFamilyIndices.graphicsFamily.value());
    queueCIs.push_back(std::move(queueCI));
  } else {
    vk::DeviceQueueCreateInfo queueCI;
    queueCI.setPQueuePriorities(&priorities)
           .setQueueCount(1)
           .setQueueFamilyIndex(m_queueFamilyIndices.graphicsFamily.value());
    queueCIs.push_back(queueCI);
    queueCI.setPQueuePriorities(&priorities)
           .setQueueCount(1)
           .setQueueFamilyIndex(m_queueFamilyIndices.presentFamily.value());
    queueCIs.push_back(queueCI);
  }

  std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  deviceCI.setQueueCreateInfos(queueCIs)
          .setPEnabledExtensionNames(deviceExtensions);

  m_device = m_phyDevice.createDevice(deviceCI);
}

void Context::getQueues() {
  m_graphicsQueue = m_device.getQueue(m_queueFamilyIndices.graphicsFamily.value(), 0);
  m_presentQueue = m_device.getQueue(m_queueFamilyIndices.presentFamily.value(), 0);
}

Context::~Context() {}

Window& Context::GetWindow() {
  return *m_window;
}

}
