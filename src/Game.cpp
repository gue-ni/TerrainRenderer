#include "Game.h"

struct Plane {
  glm::vec3 n; // plane normal
  float d; // d = dot(n, p) for a given point on the plane
  Plane(const glm::vec3 &normal, const glm::vec3 &point) : n(normal), d(glm::dot(normal, point)) {}
};

// Real Time Collison Detection - Christer Ericson
int intersect_segment_plane(glm::vec3 a, glm::vec3 b, Plane p, float &t, glm::vec3 &q)
{
  // Compute the t value for the directed line ab intersecting the plane
  glm::vec3 ab = b - a;

  t = (p.d - glm::dot(p.n, a)) / glm::dot(p.n, ab);

  // If t in [0..1] compute and return intersection point
  if (t >= 0.0f && t <= 1.0f) {
    q = a + t * ab;
    return true;
  }

  return false;
}

Game::Game(size_t width, size_t height)
    : Window(width, height), m_terrain_renderer(glm::vec2(-200.0f), glm::vec2(200.0f))
{
  SDL_ShowCursor(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  float fov = 45.0f;
  float aspect_ratio = float(width) / float(height);
  float near = 1.0f;
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

  auto lod_focus = m_camera.get_local_position();

  auto camera_position = m_camera.get_local_position();
  auto camera_direction = m_camera.transform_direction(glm::vec3(0.0f, 0.0f, -1.0f));
  auto camera_target = camera_position + camera_direction * 1000.0f;

  float t;
  glm::vec3 point;
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 20.0f, 0.0f));

  if (intersect_segment_plane(camera_position, camera_target, plane, t, point)){
    lod_focus = point;
  }

  m_terrain_renderer.render(m_camera, glm::vec2(lod_focus.x, lod_focus.z));

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

        m_camera.yaw += delta_yaw;
        m_camera.pitch -= delta_pitch;
        m_camera.pitch = std::clamp(m_camera.pitch, -89.0f, 89.0f);

        glm::vec3 front = {
            cos(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch)),
            sin(glm::radians(m_camera.pitch)),
            sin(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch)),
        };

        front = glm::normalize(front);

        glm::vec3 world_up = {0.0f, 1.0f, 0.0f};
        glm::vec3 right = glm::normalize(glm::cross(front, world_up));
        glm::vec3 up = glm::normalize(glm::cross(right, front));
        glm::vec3 position = m_camera.get_local_position();

        auto look_at = glm::lookAt(position, position + front, up);
        m_camera.set_local_transform(glm::inverse(look_at));
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

  const Uint8 *key_states = SDL_GetKeyboardState(nullptr);

  auto right = m_camera.get_local_x_axis();
  auto forward = m_camera.get_local_z_axis();
  auto position = m_camera.get_local_position();

  float speed = 0.15f;

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
