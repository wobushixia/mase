#include "Swapchain.h"
#include "Context/Context.h"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL_video.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace mase {

void Swapchain::querySwapchainInfo() {
  auto m_phyDevice = Context::GetInstance().m_phyDevice;
  auto formats = m_phyDevice.getSurfaceFormatsKHR(Context::GetInstance().GetSurface());
  m_info.format = formats[0];
  for(const auto& format : formats) {
    if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      m_info.format = format;
      break;
    }
  }

  auto capabilities = m_phyDevice.getSurfaceCapabilitiesKHR(Context::GetInstance().GetSurface()); 

  uint32_t imageCount = std::max(2u, capabilities.minImageCount);
  if (capabilities.maxImageCount != 0) {
    imageCount = std::min(imageCount, capabilities.maxImageCount);
  }
  m_info.imageCount = imageCount;

  int w = 0, h = 0;
  SDL_GetWindowSize(Context::GetInstance().GetWindow().GetWindow(), &w, &h);

  m_info.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
  m_info.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

  m_info.transform = capabilities.currentTransform;

  auto presentModes = m_phyDevice.getSurfacePresentModesKHR(Context::GetInstance().GetSurface());

  m_info.presentMode = vk::PresentModeKHR::eFifo;
  for (auto presentMode : presentModes) {
    if(presentMode == vk::PresentModeKHR::eMailbox) {
      m_info.presentMode = presentMode;
      break;
    }
  }
}

void Swapchain::createSwapchain() {
  vk::SwapchainCreateInfoKHR ci;
  ci.setClipped(true)
    .setImageArrayLayers(1)
    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
    .setSurface(Context::GetInstance().GetSurface())
    .setImageColorSpace(m_info.format.colorSpace)
    .setImageFormat(m_info.format.format)
    .setImageExtent(m_info.imageExtent)
    .setMinImageCount(m_info.imageCount);

  auto& queueIndices = Context::GetInstance().m_queueFamilyIndices;
  if(queueIndices.graphicsFamily.value() == queueIndices.presentFamily.value()) {
    ci.setQueueFamilyIndices(queueIndices.presentFamily.value())
      .setImageSharingMode(vk::SharingMode::eExclusive) ;
  } else {
    std::array indices = { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };
    ci.setQueueFamilyIndices(indices)
      .setImageSharingMode(vk::SharingMode::eConcurrent);
  }

  m_swapchain = Context::GetInstance().m_device.createSwapchainKHR(ci);
}

void Swapchain::createImageViews() {
  images = Context::GetInstance().m_device.getSwapchainImagesKHR(m_swapchain);

  imageViews.resize(images.size());
  for (size_t i = 0; i < imageViews.size(); i++) {
    vk::ImageViewCreateInfo ci;
    vk::ComponentMapping mapping;
    vk::ImageSubresourceRange range;

    range.setBaseArrayLayer(0)
         .setLayerCount(1)
         .setBaseMipLevel(0)
         .setLevelCount(1)
         .setAspectMask(vk::ImageAspectFlagBits::eColor);
      
    ci.setImage(images[i])
      .setViewType(vk::ImageViewType::e2D)
      .setComponents(mapping)
      .setFormat(m_info.format.format)
      .setSubresourceRange(range);

    imageViews[i] = Context::GetInstance().m_device.createImageView(ci);
  }
}

Swapchain::Swapchain() {
  querySwapchainInfo();
  createSwapchain();
  createImageViews();
}

Swapchain::~Swapchain() {
  for (auto& imageView : imageViews)
    Context::GetInstance().m_device.destroyImageView(imageView);

  Context::GetInstance().m_device.destroySwapchainKHR(m_swapchain);
} 

}
