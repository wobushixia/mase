#ifndef RENDERER_H
#define RENDERER_H

#include "vulkan/vulkan.hpp"
#include <stdint.h>
#include <vulkan/vulkan.hpp>

namespace mase {

class Renderer {
public:
  void Init();
  void Destroy();
  void Render();

private:
  void recordCommandBuffer(uint32_t imageInd);
  vk::CommandBuffer m_cmdBuf;

  vk::Semaphore m_renderFinish;
  vk::Semaphore m_presentComplete;
  vk::Fence m_drawFence;
};
} // namespace mase

#endif // RENDERER_H
