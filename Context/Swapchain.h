#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace mase {

class Swapchain final {
public:
  vk::SwapchainKHR m_swapchain;

  struct SwapchainInfo {
    vk::Extent2D imageExtent;
    vk::SurfaceFormatKHR format;
    vk::SurfaceTransformFlagsKHR transform;
    vk::PresentModeKHR presentMode;
    uint32_t imageCount;
  } m_info;

  void Init();
  void Destroy();
  vk::Extent2D GetExtent() { return m_info.imageExtent; }
  void transitionImageLayout(vk::CommandBuffer cmdBuf, uint32_t imageIndex,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::AccessFlags2 src_access_mask,
                             vk::AccessFlags2 dst_access_mask,
                             vk::PipelineStageFlags2 src_stage_mask,
                             vk::PipelineStageFlags2 dst_stage_mask);

  std::vector<vk::Image> m_images;
  std::vector<vk::ImageView> m_imageViews;

private:
  void querySwapchainInfo();
  void createSwapchain();
  void createImageViews();
};

} // namespace mase

#endif // !SWAPCHAIN_H
