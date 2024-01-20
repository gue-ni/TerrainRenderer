/* Base class for SDL2/OpenGL window */
#pragma once

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

class Window
{
 public:
  Window(size_t width, size_t height);
  ~Window();
  virtual void run() = 0;

 protected:
  size_t m_width, m_height;
  SDL_Window* m_window{nullptr};
  SDL_GLContext m_context{nullptr};

  bool m_quit{false};

  void read_input();

 private:
};
