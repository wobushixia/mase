#include "Renderer/Renderer.h"
#include "Context/Context.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>

namespace mase {

void Renderer::Init() {
  m_cmdBuf = Context::GetInstance().m_cmdMan.allocCommandBuffers(1)[0];

  vk::SemaphoreCreateInfo sem_ci;
  m_renderFinish = Context::GetInstance().m_device.createSemaphore(sem_ci);
  m_presentComplete = Context::GetInstance().m_device.createSemaphore(sem_ci);

  vk::FenceCreateInfo fence_ci;
  fence_ci.setFlags(vk::FenceCreateFlagBits::eSignaled);
  m_drawFence = Context::GetInstance().m_device.createFence(fence_ci);
}

void Renderer::Render() {
  auto &instance = Context::GetInstance();
  auto &device = Context::GetInstance().m_device;

  auto fenceRes = device.waitForFences(m_drawFence, VK_TRUE, UINT64_MAX);
  if (fenceRes != vk::Result::eSuccess)
    throw std::runtime_error("wait for fence FAILED");
  device.resetFences(m_drawFence);

  auto [res, imageInd] = device.acquireNextImageKHR(
      instance.m_swapchain.m_swapchain, UINT64_MAX, m_presentComplete, nullptr);

  recordCommandBuffer(imageInd);

  instance.m_queue.waitIdle(); // TODO: in flight

  vk::PipelineStageFlags waitDestinationStageMask(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

  vk::SubmitInfo submitI;
  submitI.setWaitSemaphoreCount(1)
      .setPWaitSemaphores(&m_presentComplete)
      .setPWaitDstStageMask(&waitDestinationStageMask)
      .setCommandBufferCount(1)
      .setPCommandBuffers(&m_cmdBuf)
      .setSignalSemaphoreCount(1)
      .setPSignalSemaphores(&m_renderFinish);
  instance.m_queue.submit(submitI, m_drawFence);

  vk::PresentInfoKHR presentInfoKHR;
  presentInfoKHR.setWaitSemaphoreCount(1)
      .setPWaitSemaphores(&m_renderFinish)
      .setSwapchainCount(1)
      .setPSwapchains(&instance.m_swapchain.m_swapchain)
      .setPImageIndices(&imageInd);
  res = instance.m_queue.presentKHR(presentInfoKHR);
  switch (res) {
  case vk::Result::eSuccess:
    break;
  default:
    break; // an unexpected result is returned!
  }
}

void Renderer::recordCommandBuffer(uint32_t imageInd) {
  auto &instance = Context::GetInstance();

  vk::CommandBufferBeginInfo bi{};

  m_cmdBuf.begin(bi);

  instance.m_swapchain.transitionImageLayout(
      m_cmdBuf, imageInd, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal, {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
  vk::RenderingAttachmentInfo attachmentInfo;
  attachmentInfo.setImageView(instance.m_swapchain.m_imageViews[imageInd])
      .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setClearValue(clearColor);
  vk::RenderingInfo renderingInfo;
  renderingInfo.setRenderArea({{0, 0}, instance.m_swapchain.GetExtent()})
      .setLayerCount(1)
      .setColorAttachmentCount(1)
      .setPColorAttachments(&attachmentInfo);

  m_cmdBuf.beginRendering(&renderingInfo);

  m_cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics,
                        instance.m_pipeline.m_pipeline);

  m_cmdBuf.setViewport(
      0,
      vk::Viewport(0.0f, 0.0f,
                   static_cast<float>(instance.m_swapchain.GetExtent().width),
                   static_cast<float>(instance.m_swapchain.GetExtent().height),
                   0.0f, 1.0f));
  m_cmdBuf.setScissor(
      0, vk::Rect2D(vk::Offset2D(0, 0), instance.m_swapchain.GetExtent()));

  m_cmdBuf.draw(3, 1, 0, 0);

  m_cmdBuf.endRendering();

  instance.m_swapchain.transitionImageLayout(
      m_cmdBuf, imageInd, vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite, {},
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe);

  m_cmdBuf.end();
}

void Renderer::Destroy() {
  auto &device = Context::GetInstance().m_device;
  device.destroyFence(m_drawFence);
  device.destroySemaphore(m_presentComplete);
  device.destroySemaphore(m_renderFinish);
}

} // namespace mase
