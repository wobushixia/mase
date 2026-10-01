#include "Mase/Mase.h"
#include "Context/Context.h"

void App::run() {
  Init();
  while(!mase::Context::GetInstance().GetWindowShouldClose()) {
    Render();
    mase::Context::GetInstance().Update();
  }
  Shutdown();
}

void App::Init() {
  mase::Context::GetInstance().Init();
}

void App::Shutdown() {
  mase::Context::GetInstance().Shutdown();
}

void Mase::Render() {}
