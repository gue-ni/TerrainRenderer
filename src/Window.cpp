#include "Window.h"

Window::Window(size_t width, size_t height, const std::string& name) : m_width(width), m_height(height)
{
  SDL_Init(SDL_INIT_VIDEO);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)m_width, (int)m_height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  m_context = SDL_GL_CreateContext(m_window);

  glewExperimental = GL_TRUE;
  glewInit();
}

Window::~Window()
{
  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}
