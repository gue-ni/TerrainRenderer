#include "Game.h"

Game::Game(size_t width, size_t height)
    : Window(width, height), m_terrain_renderer(glm::vec2(-100.0f), glm::vec2(100.0f))
{
  // GL_CALL(glEnable(GL_DEPTH_TEST));

  SDL_ShowCursor(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  float fov = 45.0f;
  float aspect_ratio = float(width) / float(height);
  float near = 1.01f;
  float far = 1000.0f;
  auto projection = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
  m_camera.set_projection_matrix(projection);
  m_camera.set_local_position(glm::vec3(0.0f, 40.f, 0.0f));
}

void Game::render(float dt)
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
  m_clock.init();
  while (!m_quit) {
    float dt = m_clock.delta_time();
    read_input(dt);
    update(dt);
    render(dt);
    m_clock.tick();
  }
}

void Game::read_input(float dt)
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

        m_camera.yaw -= delta_yaw;
        m_camera.pitch -= delta_pitch;
        m_camera.pitch = std::clamp(m_camera.pitch, -89.0f, 89.0f);

        //printf("%.2f, %.2f\n", m_camera.pitch, m_camera.yaw);

#if 0
        auto rotation = m_camera.get_local_rotation();
        glm::quat pitch = glm::quat(glm::radians(glm::vec3(-delta_pitch, 0.0f, 0.0f)));
        glm::quat yaw = glm::quat(glm::radians(glm::vec3(0.0f, -delta_yaw, 0.0f)));
        glm::quat new_rotation = rotation * yaw * pitch;
        m_camera.set_local_rotation(new_rotation);
#else

        glm::vec3 front(0.0f, 0.0f, 0.0f);
        front.x = cos(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch));
        front.y = sin(glm::radians(m_camera.pitch));
        front.z = sin(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch));
        front = glm::normalize(front);

        std::cout << front << std::endl;

        glm::vec3 world_up = {0.0f, 1.0f, 0.0f};
        glm::vec3 right = glm::normalize(glm::cross(front, world_up));
        glm::vec3 up = glm::normalize(glm::cross(right, front));
        glm::vec3 position = m_camera.get_local_position();

        auto look_at = glm::lookAt(position, position + front, up);
        m_camera.set_local_transform(glm::inverse(look_at));
#endif
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
  }

  const Uint8* key_states = SDL_GetKeyboardState(nullptr);

  auto right = m_camera.get_local_x_axis();
  auto forward = m_camera.get_local_z_axis();
  auto position = m_camera.get_local_position();

  float speed = 0.05f;

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
