#include "Game.h"

Game::Game(size_t width, size_t height)
    : Window(width, height), m_terrain_renderer(glm::vec2(-100.0f), glm::vec2(100.0f))
{
  // glEnable(GL_CULL_FACE);

  SDL_CaptureMouse(SDL_TRUE);

  float fov = 45.0f;
  auto projection = glm::perspective(glm::radians(fov), float(width) / float(height), 0.01f, 10000.0f);
  m_camera.set_projection_matrix(projection);
  m_camera.set_local_position(glm::vec3(0.0f, 10.f, 0.0f));
}

void Game::render()
{
  glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto camera_position = m_camera.get_local_position();
  m_terrain_renderer.render(m_camera, glm::vec2(camera_position.x, camera_position.z));

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
    switch (sdl_event.type) {
      case SDL_QUIT: {
        m_quit = true;
        break;
      }
      case SDL_MOUSEMOTION: {
        const float sensitivity = 0.10f;
        float delta_yaw = static_cast<float>(sdl_event.motion.xrel) * sensitivity;
        float delta_pitch = static_cast<float>(sdl_event.motion.yrel) * sensitivity;

        glm::quat rotate_by = glm::quat(glm::radians(glm::vec3(-delta_pitch, -delta_yaw, 0.0f)));
        glm::quat new_rotation = m_camera.get_local_rotation() * rotate_by;
        m_camera.set_local_rotation(new_rotation);
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        break;
      }
      case SDL_KEYDOWN: {
        break;
      }
    }

    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
      if (sdl_event.key.keysym.sym == SDLK_SPACE) {
        m_terrain_renderer.wireframe = !m_terrain_renderer.wireframe;
      }

      if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
        m_quit = true;
      }
    }

    if (sdl_event.type == SDL_MOUSEBUTTONDOWN) {
      if (sdl_event.button.button == SDL_BUTTON_LEFT) {
        glm::vec2 click;
        click.x = (float)sdl_event.button.x / (float)m_width;
        click.y = (float)sdl_event.button.y / (float)m_height;

        std::cout << click << std::endl;
        // [0, 1] -> [-1, 1]
        m_last_click_location = glm::vec2(-1.0f) + click * (glm::vec2(1.0f) - glm::vec2(-1.0f));
        m_last_click_location = -glm::vec2(-m_last_click_location.x, m_last_click_location.y);

        // std::cout << click << ", " << m_last_click_location << std::endl;
      }
    }
  }

  const Uint8* key_states = SDL_GetKeyboardState(nullptr);

  auto right = m_camera.get_local_x_axis();
  auto forward = m_camera.get_local_z_axis();
  auto position = m_camera.get_local_position();

  float speed = 0.01f;

  if (key_states[SDL_SCANCODE_W]) {
    position -= forward * speed;
  }
  if (key_states[SDL_SCANCODE_A]) {
    position -= right * speed;
  }
  if (key_states[SDL_SCANCODE_S]) {
    position += forward * speed;
  }
  if (key_states[SDL_SCANCODE_D]) {
    position += right * speed;
  }

  m_camera.set_local_position(position);
}

void Game::update(float dt)
{
  // TODO: update camera position
}
