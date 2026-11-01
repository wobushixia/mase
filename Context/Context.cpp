#include "Context/Context.h"
#include "Base/File.h"
#include "Context/Shader.h"
#include "Context/Swapchain.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace mase {

Context::Context() {}

void Context::Shutdown() {
  m_device.waitIdle();

  m_renderer.Destroy();
  m_cmdMan.Destroy();
  m_pipeline.Destroy();
  m_shader.Destroy();
  m_swapchain.Destroy();
  m_device.destroy();
  m_instance.destroySurfaceKHR(m_surface);
  m_instance.destroy();
  m_window.Destroy();
  SDL_Quit();
}

void Context::Update() {
  while (SDL_PollEvent(&m_event)) {
    switch (m_event.type) {
    case SDL_EVENT_QUIT:
      m_windowShouldClose = true;
      break;
    default:
      break;
    }
  }

  m_renderer.Render();
  m_device.waitIdle();
}

void Context::Init() {
  createWindow();
  createInstance();
  createSurface();
  pickupPhysicalDevice();
  queryQueueFamily();
  createDevice();
  getQueues();
  createSwapchain();
  createShaders();
  createPipeline();
  createCommandManager();
  createRenderer();
}

void Context::createRenderer() { m_renderer.Init(); }

void Context::createWindow() { m_window.Init(); }

void Context::createCommandManager() { m_cmdMan.Init(); }

void Context::createPipeline() { m_pipeline.Init(); }

void Context::createSwapchain() { m_swapchain.Init(); }

Context &Context::GetInstance() {
  static Context instance;
  return instance;
}

void Context::createSurface() {
  VkSurfaceKHR _surface;

  if (!SDL_Vulkan_CreateSurface(m_window.GetWindow(), m_instance, nullptr,
                                &_surface))
    throw std::runtime_error("create SDL3 surface FAILED");
  m_surface = vk::SurfaceKHR(_surface);
}

void Context::createInstance() {
  vk::InstanceCreateInfo ci;
  const std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

  Uint32 sdlExtCount = 0;
  const char *const *sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
  std::vector extensions(sdlExts, sdlExts + sdlExtCount);

  vk::ApplicationInfo ai;
  ai.setApiVersion(VK_API_VERSION_1_4);
  ci.setPApplicationInfo(&ai)
      .setPEnabledLayerNames(layers)
      .setPEnabledExtensionNames(extensions);

  m_instance = vk::createInstance(ci);
}

bool isDeviceSuitable(vk::PhysicalDevice const &device) {
  auto features = device.getFeatures();

  if (features.geometryShader)
    return true;

  return false;
}

void Context::pickupPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices();
  m_phyDevice = devices[0];

  for (auto device : devices) {
    if (isDeviceSuitable(device))
      m_phyDevice = device;
    break;
  }
}

void Context::queryQueueFamily() {
  auto queueFamilies = m_phyDevice.getQueueFamilyProperties();

  for (size_t i = 0; i < queueFamilies.size(); i++) {
    const auto &queueFamily = queueFamilies[i];

    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
        m_phyDevice.getSurfaceSupportKHR(i, m_surface)) {
      m_qfpIndex = i;
      break;
    }
  }
}

void Context::createDevice() {
  vk::DeviceCreateInfo deviceCI;
  std::vector<vk::DeviceQueueCreateInfo> queueCIs;

  float priorities = 1.0f;

  vk::DeviceQueueCreateInfo queueCI;
  queueCI.setPQueuePriorities(&priorities)
      .setQueueCount(1)
      .setQueueFamilyIndex(m_qfpIndex);
  queueCIs.push_back(std::move(queueCI));

  std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  deviceCI.setQueueCreateInfos(queueCIs).setPEnabledExtensionNames(
      deviceExtensions);

  vk::PhysicalDeviceVulkan11Features features11;
  features11.setShaderDrawParameters(true);
  deviceCI.pNext = &features11;

  vk::PhysicalDeviceDynamicRenderingFeatures dynFeatures;
  dynFeatures.setDynamicRendering(true);
  features11.pNext = &dynFeatures;

  vk::PhysicalDeviceSynchronization2Features sync2Features{};
  sync2Features.synchronization2 = VK_TRUE;
  dynFeatures.pNext = &sync2Features;

  m_device = m_phyDevice.createDevice(deviceCI);
}

void Context::getQueues() { m_queue = m_device.getQueue(m_qfpIndex, 0); }

void Context::createShaders() {
  File shaderFile;
  if (!shaderFile.OpenBinary("../Assets/shaders/slang.spv"))
    throw std::runtime_error("Failed to open file");
  std::string shaderCode;
  shaderFile.ReadAll(&shaderCode);
  m_shader.Create(shaderCode);
}

Context::~Context() {}

Window &Context::GetWindow() { return m_window; }

} // namespace mase
