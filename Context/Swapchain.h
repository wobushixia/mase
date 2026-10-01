#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan/vulkan.hpp"

namespace mase {

class Swapchain final {
public:
  vk::SwapchainKHR m_swapchain;

  Swapchain();
  ~Swapchain();

private:
  struct SwapchainInfo {
    vk::Extent2D imageExtent;
    vk::SurfaceFormatKHR format;
    vk::SurfaceTransformFlagsKHR transform;
    vk::PresentModeKHR presentMode;
    uint32_t imageCount;
  } m_info;

  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imageViews;

  void querySwapchainInfo();
  void createSwapchain();
  void createImageViews();
};

}

#endif // !SWAPCHAIN_H

