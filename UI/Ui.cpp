#include "Context/Context.h"
#include "imgui.h"
#include <UI/Ui.h>

namespace mase {

void UiManager::createImGuiPool() {
  constexpr vk::DescriptorPoolSize pool_sizes[] = {
      {vk::DescriptorType::eSampler, 2},
      {vk::DescriptorType::eSampledImage, 2},
      {vk::DescriptorType::eUniformBuffer, 2},
  };

  vk::DescriptorPoolCreateInfo poolInfo{};
  poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  poolInfo.maxSets = 4;
  poolInfo.poolSizeCount = std::size(pool_sizes);
  poolInfo.pPoolSizes = pool_sizes;

  m_imguiPool = Context::GetInstance().m_device.createDescriptorPool(poolInfo);
}

void UiManager::initImGui() {
  auto &instance = Context::GetInstance();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  // io.Fonts->AddFontFromFileTTF("../font/mgenplus-1mn-bold.ttf", 18.0f,
  // nullptr,
  //                              io.Fonts->GetGlyphRangesJapanese());
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  ImGui::GetStyle().ScaleAllSizes(main_scale);
  ImGui::GetStyle().FontScaleDpi = main_scale;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForVulkan(instance.m_window.GetWindow());

  vk::PipelineRenderingCreateInfoKHR dyn{};
  vk::Format colorFormat = instance.m_swapchain.m_info.format.format;
  dyn.setColorAttachmentCount(1).setPColorAttachmentFormats(&colorFormat);

  ImGui_ImplVulkan_InitInfo vi{};
  vi.Instance = instance.m_instance;
  vi.PhysicalDevice = instance.m_phyDevice;
  vi.Device = instance.m_device;
  vi.QueueFamily = instance.m_qfpIndex;
  vi.Queue = instance.m_queue;
  vi.DescriptorPool = m_imguiPool;
  vi.MinImageCount = 2;
  vi.ImageCount =
      static_cast<uint32_t>(instance.m_swapchain.m_imageViews.size());
  vi.UseDynamicRendering = true;
  vi.ApiVersion = VK_API_VERSION_1_4;
  vi.PipelineInfoMain.PipelineRenderingCreateInfo = dyn;

  ImGui_ImplVulkan_Init(&vi);
}

void UiManager::Init() {
  createImGuiPool();
  initImGui();
}

void UiManager::Destroy() {
  auto &instance = Context::GetInstance();

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  instance.m_device.destroyDescriptorPool(m_imguiPool);
}

void UiManager::Begin() {
  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplVulkan_NewFrame();
  ImGui::NewFrame();
}

void UiManager::End() { ImGui::Render(); }
}; // namespace mase
