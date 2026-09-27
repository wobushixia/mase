#include "Context/Context.h"
#include "Renderer/Renderer.h"
#include <SDL3/SDL_render.h>
#include <memory>

int main() {
  auto ctx = std::make_unique<mase::Context>();

  ctx->Init();
  SDL_Renderer* renderer = ctx->GetWindow().GetRenderer();

  while(!ctx->GetWindowShouldClose()) {
    SDL_SetRenderDrawColor(renderer, 57, 197, 187, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    ctx->Update();
  }

  ctx->Shutdown();

  return 0;
}
