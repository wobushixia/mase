#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>
#include <vector>
#include "vulkan/vulkan.hpp"

namespace mase {

class CommandManager {
public:
  vk::CommandPool m_cmdPool;

  void Init();
  void Destroy();
  std::vector<vk::CommandBuffer> allocCommandBuffers(std::uint32_t count);
  vk::CommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(vk::CommandBuffer &cmdBuf);
private:
  void createCommandPool();
};

}

#endif // !COMMAND_H
