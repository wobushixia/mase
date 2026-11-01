#include "Context/Command.h"
#include "Context/Context.h"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace mase {

void CommandManager::Init() {
  createCommandPool();
}

void CommandManager::Destroy() {
  Context::GetInstance().m_device.destroyCommandPool(m_cmdPool);
}

void CommandManager::createCommandPool() {
  vk::CommandPoolCreateInfo ci;

  ci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
    .setQueueFamilyIndex(Context::GetInstance().m_qfpIndex);

  m_cmdPool = Context::GetInstance().m_device.createCommandPool(ci);
}

std::vector<vk::CommandBuffer> CommandManager::allocCommandBuffers(std::uint32_t count) {
  auto& ctx = Context::GetInstance();

  vk::CommandBufferAllocateInfo ai;
  ai.setCommandPool(m_cmdPool)
    .setCommandBufferCount(count)
    .setLevel(vk::CommandBufferLevel::ePrimary);

  return ctx.m_device.allocateCommandBuffers(ai);
}

vk::CommandBuffer CommandManager::BeginSingleTimeCommands() {
  return allocCommandBuffers(1)[0];
}

void CommandManager::EndSingleTimeCommands(vk::CommandBuffer &cmdBuf) {}

}
