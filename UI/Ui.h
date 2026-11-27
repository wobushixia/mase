#ifndef UI_H
#define UI_H

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "vulkan/vulkan.hpp"
#include <imgui.h>

namespace mase {

class UiManager {

private:
  vk::DescriptorPool m_imguiPool;

  void createImGuiPool();
  void initImGui();

public:
  void Init();
  void Destroy();
  void Begin();
  void End();
};

} // namespace mase

#endif //! UI_H
