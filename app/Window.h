/* Base class for SDL2/OpenGL window */
#pragma once

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <imgui.h>

#include <string>

class Window
{
 public:
  Window(size_t width, size_t height, const std::string& name = "TerrainRenderer");
  ~Window();
  float aspect_ratio() const;
  void resize(size_t width, size_t height);

 protected:
  size_t m_width, m_height;
  SDL_Window* m_window{nullptr};
  SDL_GLContext m_context{nullptr};
};
