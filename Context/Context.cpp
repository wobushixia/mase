#define VMA_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "Context/Context.h"
#include "Base/File.h"
#include "Context/Shader.h"
#include "Context/Swapchain.h"
#include "Context/tiny_obj_loader.h"
#include "Window/Window.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace mase {

struct ShaderData {
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model[3];
  glm::vec4 lightPos{0.0f, -10.0f, 10.0f, 0.0f};
} shaderData{};

Context::Context() {}

void Context::Shutdown() {
  m_device.waitIdle();

  vmaDestroyAllocator(m_allocator);
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
  initVma();
  createDepthBuffer();
  loadModel();
}

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;
};

void Context::loadModel() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr,
                   "../Assets/models/model.obj");

  const VkDeviceSize indexCount{shapes[0].mesh.indices.size()};
  std::vector<Vertex> vertices{};
  std::vector<uint16_t> indices{};

  for (auto &index : shapes[0].mesh.indices) {
    Vertex v{.pos = {attrib.vertices[index.vertex_index * 3],
                     -attrib.vertices[index.vertex_index * 3 + 1],
                     attrib.vertices[index.vertex_index * 3 + 2]},
             .normal = {attrib.normals[index.normal_index * 3],
                        -attrib.normals[index.normal_index * 3 + 1],
                        attrib.normals[index.normal_index * 3 + 2]},
             .uv = {attrib.texcoords[index.texcoord_index * 2],
                    1.0 - attrib.texcoords[index.texcoord_index * 2 + 1]}};
    vertices.push_back(v);
    indices.push_back(indices.size());
  }

  VkDeviceSize vBufSize{sizeof(Vertex) * vertices.size()};
  VkDeviceSize iBufSize{sizeof(uint16_t) * indices.size()};
  VkBufferCreateInfo bufferCI{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                              .size = vBufSize + iBufSize,
                              .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT};

  VmaAllocationCreateInfo vBufferAllocCI{
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
               VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO};
  VmaAllocationInfo vBufferAllocInfo{};
  vmaCreateBuffer(m_allocator, &bufferCI, &vBufferAllocCI, &vBuffer.buffer,
                  &vBuffer.allocation, &vBufferAllocInfo);

  memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
  memcpy(((char *)vBufferAllocInfo.pMappedData) + vBufSize, indices.data(),
         iBufSize);
}

void Context::createDepthBuffer() {
  std::vector<VkFormat> depthFormatList{VK_FORMAT_D32_SFLOAT_S8_UINT,
                                        VK_FORMAT_D24_UNORM_S8_UINT};
  VkFormat depthFormat{VK_FORMAT_UNDEFINED};
  for (VkFormat &format : depthFormatList) {
    VkFormatProperties2 formatProperties{
        .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    vkGetPhysicalDeviceFormatProperties2(m_phyDevice, format,
                                         &formatProperties);
    if (formatProperties.formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      depthFormat = format;
      break;
    }
  }

  VkImageCreateInfo depthImageCI{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = depthFormat,
      .extent{.width = static_cast<uint32_t>(m_swapchain.GetExtent().width),
              .height = static_cast<uint32_t>(m_swapchain.GetExtent().height),
              .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo allocCI{
      .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO};
  vmaCreateImage(m_allocator, &depthImageCI, &allocCI, &m_depth.img,
                 &m_depth.allocation, nullptr);

  VkImageViewCreateInfo depthViewCI{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = m_depth.img,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = depthFormat,
      .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                        .levelCount = 1,
                        .layerCount = 1}};
  vkCreateImageView(m_device, &depthViewCI, nullptr, &m_depth.imgView);
}

void Context::initVma() {
  VmaVulkanFunctions vkFunctions{.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                                 .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                                 .vkCreateImage = vkCreateImage};
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
