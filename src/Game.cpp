#include "Game.h"

Game::Game(size_t width, size_t height) : Window(width, height)
{
  glEnable(GL_CULL_FACE);

  float fov = 45.0f;
  auto projection = glm::perspective(glm::radians(fov), float(width) / float(height), 0.01f, 100.0f);
  m_camera.set_projection_matrix(projection);
  m_camera.set_local_position(glm::vec3(0.0f, 0.f, 3.0f));
}

void Game::render()
{
  glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto terrain_center = glm::vec2(0.0f);

  m_terrain_renderer.render(m_camera, terrain_center);

  SDL_GL_SwapWindow(m_window);
}

void Game::run()
{
  float dt = 0.01f;
  while (!m_quit) {
    read_input();
    update(dt);
    render();
  }
}

void Game::read_input()
{
  SDL_Event sdl_event;
  while (SDL_PollEvent(&sdl_event) != 0) {
    if (sdl_event.type == SDL_QUIT) {
      m_quit = true;
      break;
    }
  }
}

void Game::update(float dt)
{
  // TODO: update camera position
}
