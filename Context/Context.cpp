#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define VMA_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "Base/File.h"
#include "Context/Context.h"
#include "Context/Shader.h"
#include "Context/Swapchain.h"
#include "Context/tiny_obj_loader.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_vulkan.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace mase {

Context::Context() {}

void Context::Shutdown() {
  m_device.waitIdle();

  if (m_depth.imgView) {
    m_device.destroyImageView(m_depth.imgView);
    m_depth.imgView = VK_NULL_HANDLE;
  }

  if (m_depth.img != VK_NULL_HANDLE) {
    vmaDestroyImage(m_allocator, m_depth.img, m_depth.allocation);
    m_depth.img = VK_NULL_HANDLE;
    m_depth.allocation = VK_NULL_HANDLE;
  }

  if (vBuffer.buffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(m_allocator, vBuffer.buffer, vBuffer.allocation);
    vBuffer.buffer = VK_NULL_HANDLE;
  }

  for (auto &sd : m_SDBufs) {
    if (sd.buf != VK_NULL_HANDLE) {
      vmaDestroyBuffer(m_allocator, sd.buf, sd.alloc);
      sd.buf = VK_NULL_HANDLE;
    }
  }

  vmaDestroyAllocator(m_allocator);

  m_uiMan.Destroy();
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
    ImGui_ImplSDL3_ProcessEvent(&m_event);
    switch (m_event.type) {
    case SDL_EVENT_QUIT:
      m_windowShouldClose = true;
      break;
    default:
      break;
    }
  }

  RenderUi();
  m_renderer.Render();
}

void Context::RenderUi() {
  m_uiMan.Begin();
  ImGui::Begin("Mase");
  static float t = 0.0f;
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  ImGui::End();
  m_uiMan.End();
}

void Context::Init() {
  createWindow();
  createInstance();
  createSurface();
  pickupPhysicalDevice();
  queryQueueFamily();
  createDevice();
  getQueues();
  initVma();
  createSwapchain();
  createCommandManager();
  createDepthBuffer();
  createShaders();
  createPipeline();
  createRenderer();
  loadModel();
  createSDBuffers();
  updateSDBuffers();
  createUiManager();
}

void Context::createUiManager() { m_uiMan.Init(); }

void Context::createSDBuffers() {
  m_SDBufs.resize(MAX_FRAMES_IN_FLIGHT);
  for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkBufferCreateInfo uBufferCI{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .size = sizeof(ShaderData),
                                 .usage =
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT};

    VmaAllocationCreateInfo uBufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO};

    vmaCreateBuffer(m_allocator, &uBufferCI, &uBufferAllocCI, &m_SDBufs[i].buf,
                    &m_SDBufs[i].alloc, &m_SDBufs[i].allocInfo);

    VkBufferDeviceAddressInfo uBufferBdaInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = m_SDBufs[i].buf};
    m_SDBufs[i].deviceAddr =
        vkGetBufferDeviceAddress(m_device, &uBufferBdaInfo);
  }
}

void Context::updateSDBuffers() {
  const float aspect = static_cast<float>(m_swapchain.GetExtent().width) /
                       static_cast<float>(m_swapchain.GetExtent().height);

  ShaderData sd{};
  sd.projection = glm::perspective(glm::radians(45.0f),
                                   (float)m_swapchain.GetExtent().width /
                                       (float)m_swapchain.GetExtent().height,
                                   0.1f, 32.0f);
  sd.view =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));
  auto instancePos = glm::vec3(0.0f, 0.0f, 0.0f);
  sd.model = glm::mat4(1.0f);

  for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    memcpy(m_SDBufs[i].allocInfo.pMappedData, &sd, sizeof(ShaderData));
    vmaFlushAllocation(m_allocator, m_SDBufs[i].alloc, 0, sizeof(ShaderData));
  }
}

void Context::loadModel() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        "../Assets/models/miku.obj")) {
    throw std::runtime_error("load obj failed: " + warn + err);
  }

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (auto &shape : shapes) {
    for (auto &index : shape.mesh.indices) {
      Vertex v{};
      v.pos = {attrib.vertices[index.vertex_index * 3],
               -attrib.vertices[index.vertex_index * 3 + 1],
               attrib.vertices[index.vertex_index * 3 + 2]};
      if (index.normal_index >= 0) {
        v.normal = {attrib.normals[index.normal_index * 3],
                    -attrib.normals[index.normal_index * 3 + 1],
                    attrib.normals[index.normal_index * 3 + 2]};
      }
      if (index.texcoord_index >= 0) {
        v.uv = {attrib.texcoords[index.texcoord_index * 2],
                1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]};
      }
      indices.push_back(static_cast<uint32_t>(vertices.size()));
      vertices.push_back(v);
    }
  }

  m_indexCount = static_cast<uint32_t>(indices.size());

  VkDeviceSize vBufSize = sizeof(Vertex) * vertices.size();
  VkDeviceSize iBufSize = sizeof(uint32_t) * indices.size();

  VkBufferCreateInfo bufferCI{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                              .size = vBufSize + iBufSize,
                              .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT};

  VmaAllocationCreateInfo vBufferAllocCI{
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO};

  VmaAllocationInfo vBufferAllocInfo{};
  vmaCreateBuffer(m_allocator, &bufferCI, &vBufferAllocCI, &vBuffer.buffer,
                  &vBuffer.allocation, &vBufferAllocInfo);

  memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
  memcpy(reinterpret_cast<char *>(vBufferAllocInfo.pMappedData) + vBufSize,
         indices.data(), iBufSize);

  vmaFlushAllocation(m_allocator, vBuffer.allocation, 0, VK_WHOLE_SIZE);

  vBuffer.vertexOffset = 0;
  vBuffer.indexOffset = vBufSize;
}

void Context::createDepthBuffer() {
  std::vector<vk::Format> depthFormatList{vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint};
  vk::Format depthFormat = vk::Format::eUndefined;
  for (auto format : depthFormatList) {
    auto props = m_phyDevice.getFormatProperties(format);
    if (props.optimalTilingFeatures &
        vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
      depthFormat = format;
      break;
    }
  }
  if (depthFormat == vk::Format::eUndefined)
    throw std::runtime_error("no depth format available");

  m_depth.format = VkFormat(depthFormat);

  auto extent = m_swapchain.GetExtent();

  vk::ImageCreateInfo depthImageCI;
  depthImageCI.setImageType(vk::ImageType::e2D)
      .setFormat(depthFormat)
      .setExtent({extent.width, extent.height, 1})
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setTiling(vk::ImageTiling::eOptimal)
      .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
      .setInitialLayout(vk::ImageLayout::eUndefined);

  VmaAllocationCreateInfo allocCI{};
  allocCI.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  allocCI.usage = VMA_MEMORY_USAGE_AUTO;

  VkImage rawImage = VK_NULL_HANDLE;
  vmaCreateImage(m_allocator,
                 reinterpret_cast<const VkImageCreateInfo *>(&depthImageCI),
                 &allocCI, &rawImage, &m_depth.allocation, nullptr);
  m_depth.img = vk::Image(rawImage);

  vk::ImageViewCreateInfo depthViewCI;
  depthViewCI.setImage(m_depth.img)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(depthFormat)
      .setSubresourceRange(vk::ImageSubresourceRange{}
                               .setAspectMask(vk::ImageAspectFlagBits::eDepth |
                                              vk::ImageAspectFlagBits::eStencil)
                               .setBaseMipLevel(0)
                               .setLevelCount(1)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1));
  m_depth.imgView = m_device.createImageView(depthViewCI);

  auto cmd = m_cmdMan.BeginSingleTimeCommands();

  vk::ImageAspectFlags depthAspect =
      vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
  m_swapchain.transitionImageLayout(
      cmd, m_depth.img, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal, depthAspect,
      vk::AccessFlagBits2::eNone,
      vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
      vk::PipelineStageFlagBits2::eTopOfPipe,
      vk::PipelineStageFlagBits2::eEarlyFragmentTests);

  m_cmdMan.EndSingleTimeCommands(cmd);
}

void Context::initVma() {
  VmaVulkanFunctions vkFunctions{.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                                 .vkGetDeviceProcAddr = vkGetDeviceProcAddr};
  VmaAllocatorCreateInfo allocatorCI{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = m_phyDevice,
      .device = m_device,
      .pVulkanFunctions = &vkFunctions,
      .instance = m_instance};
  vmaCreateAllocator(&allocatorCI, &m_allocator);
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
  std::vector<const char *> extensions(sdlExts, sdlExts + sdlExtCount);

  vk::ApplicationInfo ai;
  ai.setApiVersion(VK_API_VERSION_1_4);
  ci.setPApplicationInfo(&ai)
      .setPEnabledLayerNames(layers)
      .setPEnabledExtensionNames(extensions);

  m_instance = vk::createInstance(ci);
}

bool isDeviceSuitable(vk::PhysicalDevice const &device) {
  auto features = device.getFeatures();
  if (!features.geometryShader)
    return false;

  auto exts = device.enumerateDeviceExtensionProperties();
  bool hasSwapchain = false;
  for (auto &e : exts) {
    if (std::strcmp(e.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      hasSwapchain = true;
      break;
    }
  }
  if (!hasSwapchain)
    return false;

  return true;
}

void Context::pickupPhysicalDevice() {
  auto devices = m_instance.enumeratePhysicalDevices();
  if (devices.empty())
    throw std::runtime_error("no Vulkan device");

  m_phyDevice = devices[0];
  bool found = false;
  for (auto &device : devices) {
    if (isDeviceSuitable(device)) {
      m_phyDevice = device;
      found = true;
      break;
    }
  }
  if (!found)
    throw std::runtime_error("no suitable Vulkan device");
}

void Context::queryQueueFamily() {
  auto queueFamilies = m_phyDevice.getQueueFamilyProperties();
  m_qfpIndex = UINT32_MAX;

  for (uint32_t i = 0; i < queueFamilies.size(); i++) {
    const auto &queueFamily = queueFamilies[i];
    if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) &&
        m_phyDevice.getSurfaceSupportKHR(i, m_surface)) {
      m_qfpIndex = i;
      break;
    }
  }
  if (m_qfpIndex == UINT32_MAX)
    throw std::runtime_error("no suitable queue family");
}

void Context::createDevice() {
  vk::DeviceCreateInfo deviceCI;
  std::vector<vk::DeviceQueueCreateInfo> queueCIs;

  float priorities = 1.0f;
  vk::DeviceQueueCreateInfo queueCI;
  queueCI.setPQueuePriorities(&priorities)
      .setQueueCount(1)
      .setQueueFamilyIndex(m_qfpIndex);
  queueCIs.push_back(queueCI);

  std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  vk::PhysicalDeviceVulkan12Features features12;
  features12.setBufferDeviceAddress(true);

  vk::PhysicalDeviceDynamicRenderingFeatures dynFeatures;
  dynFeatures.setDynamicRendering(true);
  features12.pNext = &dynFeatures;

  vk::PhysicalDeviceSynchronization2Features sync2Features;
  sync2Features.setSynchronization2(true);
  dynFeatures.pNext = &sync2Features;

  deviceCI.setQueueCreateInfos(queueCIs)
      .setPEnabledExtensionNames(deviceExtensions)
      .setPNext(&features12);

  m_device = m_phyDevice.createDevice(deviceCI);
}

void Context::getQueues() { m_queue = m_device.getQueue(m_qfpIndex, 0); }

void Context::createShaders() {
  File shaderFile;
  if (!shaderFile.OpenBinary("../Assets/shaders/slang.spv"))
    throw std::runtime_error("Failed to open shader file");
  std::string shaderCode;
  shaderFile.ReadAll(&shaderCode);
  m_shader.Create(shaderCode);
}

Context::~Context() {}

Window &Context::GetWindow() { return m_window; }

} // namespace mase
