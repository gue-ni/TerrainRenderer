#include "Game.h"

Game::Game(size_t width, size_t height) : Window(width, height) {}

void Game::render()
{
  glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  SDL_GL_SwapWindow(m_window);
}

void Game::run()
{
  while (!m_quit) {
    read_input();
    render();
  }
}
