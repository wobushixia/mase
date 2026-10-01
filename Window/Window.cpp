#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include "Window.h"

mase::Window::Window() {
  if(!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL: SDL_INIT FAILED, %s", SDL_GetError());
    return;
  }

  m_window = nullptr;
  m_renderer = nullptr;

  if(!SDL_CreateWindowAndRenderer("mase", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_VULKAN, &m_window, &m_renderer)) SDL_Log("%s", SDL_GetError());
}

mase::Window::~Window() {
  SDL_DestroyWindow(m_window);
  SDL_DestroyRenderer(m_renderer);
}

SDL_Window* mase::Window::GetWindow() {
  return m_window;
}

