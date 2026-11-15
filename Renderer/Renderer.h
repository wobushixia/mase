#ifndef RENDERER_H
#define RENDERER_H

#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan.hpp>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

namespace mase {

class Renderer {
public:
  void Init();
  void Destroy();
  void Render();

  void DrawLine(glm::vec3 p1, glm::vec3 p2);

private:
  void recordCommandBuffer(uint32_t imageInd);
  std::vector<vk::CommandBuffer> m_cmdBufs;

  std::vector<vk::Semaphore> m_renderFinishSems;
  std::vector<vk::Semaphore> m_presentCompleteSems;
  std::vector<vk::Fence> m_drawFences;

  uint32_t m_frameInd = 0;
};
} // namespace mase

#endif // RENDERER_H
