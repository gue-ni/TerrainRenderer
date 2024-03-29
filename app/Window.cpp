#include "Window.h"

#include "../gfx/gl.h"

Window::Window(size_t width, size_t height, const std::string& name) : m_width(width), m_height(height)
{
  SDL_Init(SDL_INIT_VIDEO);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)m_width, (int)m_height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  m_context = SDL_GL_CreateContext(m_window);

  glewExperimental = GL_TRUE;
  glewInit();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
  ImGui_ImplOpenGL3_Init();

  GL_CALL(glEnable(GL_DEPTH_TEST));

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);

  glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
}

Window::~Window()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

float Window::aspect_ratio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

void Window::resize(size_t width, size_t height)
{
  m_width = width;
  m_height = height;
  glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
}
