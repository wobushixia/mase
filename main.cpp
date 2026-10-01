#include "Mase/Mase.h"
#include <SDL3/SDL_render.h>
#include <vulkan/vulkan.hpp>

int main() {
  auto& mase = Mase::GetInstance();

  mase.run();

  return 0;
}

