#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

namespace mase {
class Window {
public:
  Window();
  ~Window();
  SDL_Window* GetWindow();
  SDL_Renderer* GetRenderer();
private:
  SDL_Window* m_window;
  SDL_Renderer* m_renderer;
};
}

#endif //WINDOW_H
