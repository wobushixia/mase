#include "Context/Pipeline.h"
#include "Context/Context.h"
#include "vulkan/vulkan.hpp"
#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace mase {

void Pipeline::Init() {
  vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                     vk::PipelineRenderingCreateInfo>
      chain;
  auto &ci = chain.get<vk::GraphicsPipelineCreateInfo>();
  auto &renderingCI = chain.get<vk::PipelineRenderingCreateInfo>();

  vk::PipelineVertexInputStateCreateInfo vertexCI{};
  ci.setPVertexInputState(&vertexCI);

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
  inputAssemblyCI.setPrimitiveRestartEnable(false).setTopology(
      vk::PrimitiveTopology::eTriangleList);
  ci.setPInputAssemblyState(&inputAssemblyCI);

  auto vertexShaderModule = Context::GetInstance().m_shader.GetShaderModule();
  auto fragmentShaderModule = Context::GetInstance().m_shader.GetShaderModule();
  vk::PipelineShaderStageCreateInfo vertexShaderCI{}, fragmentShaderCI{};
  vertexShaderCI.setModule(vertexShaderModule)
      .setPName("vertMain")
      .setStage(vk::ShaderStageFlagBits::eVertex);
  fragmentShaderCI.setModule(fragmentShaderModule)
      .setPName("fragMain")
      .setStage(vk::ShaderStageFlagBits::eFragment);
  std::array stagesCI{vertexShaderCI, fragmentShaderCI};
  ci.setStages(stagesCI);

  vk::PipelineViewportStateCreateInfo viewportCI{};
  vk::Viewport viewport(
      0, 0, Context::GetInstance().m_swapchain.GetExtent().width,
      Context::GetInstance().m_swapchain.GetExtent().height, 0, 1);
  vk::Rect2D scissor(
      vk::Rect2D({0, 0, Context::GetInstance().m_swapchain.GetExtent().width,
                  Context::GetInstance().m_swapchain.GetExtent().height}));
  viewportCI.setPScissors(&scissor)
      .setPViewports(&viewport)
      .setScissorCount(1)
      .setViewportCount(1);
  ci.setPViewportState(&viewportCI);

  vk::PipelineRasterizationStateCreateInfo rasterizationCI{};
  rasterizationCI.setCullMode(vk::CullModeFlagBits::eNone)
      .setRasterizerDiscardEnable(false)
      .setFrontFace(vk::FrontFace::eCounterClockwise)
      .setPolygonMode(vk::PolygonMode::eFill)
      .setLineWidth(1.0f);
  ci.setPRasterizationState(&rasterizationCI);

  vk::PipelineMultisampleStateCreateInfo msCI{};
  msCI.setSampleShadingEnable(false).setRasterizationSamples(
      vk::SampleCountFlagBits::e1);
  ci.setPMultisampleState(&msCI);

  vk::PipelineColorBlendStateCreateInfo blendCI{};
  vk::PipelineColorBlendAttachmentState attachs{};
  attachs.setBlendEnable(false).setColorWriteMask(
      vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR |
      vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB);
  std::array attachments{attachs};
  blendCI.setLogicOpEnable(false).setAttachments(attachments);
  ci.setPColorBlendState(&blendCI);

  vk::PipelineLayoutCreateInfo layoutCI{};
  layoutCI.setSetLayoutCount(0).setPushConstantRangeCount(0);
  m_pipelineLayout =
      Context::GetInstance().m_device.createPipelineLayout(layoutCI);
  if (!m_pipelineLayout) {
    throw std::runtime_error("create pipeline layout FAILED");
  }
  ci.setLayout(m_pipelineLayout);

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.setDynamicStateCount(dynamicStates.size())
      .setPDynamicStates(dynamicStates.data());
  ci.setPDynamicState(&dynamicState);

  renderingCI.setColorAttachmentCount(1).setPColorAttachmentFormats(
      &Context::GetInstance().m_swapchain.m_info.format.format);

  auto res =
      Context::GetInstance().m_device.createGraphicsPipeline(nullptr, ci);
  if (res.result != vk::Result::eSuccess) {
    throw std::runtime_error("create graphics pipeline FAILED");
  }

  m_pipeline = res.value;
}

void Pipeline::Destroy() {
  Context::GetInstance().m_device.destroyPipelineLayout(m_pipelineLayout);
  Context::GetInstance().m_device.destroyPipeline(m_pipeline);
}

} // namespace mase
