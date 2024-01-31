#include "Game.h"

#include "Collision.h"

#define INTERSECT_PLANE 1

const TileId BLUDENZ = wms::tile_id(47.1599f, 9.8082f, 9);

const TileId GROSS_GLOCKNER = wms::tile_id(47.0742f, 12.6947f, 9);

const TileId SCHNEEBERG = wms::tile_id(47.7671f, 15.8056f, 9);

const TileId INNSBRUCK = wms::tile_id(47.2692f, 11.4041f, 9);

const TileId VIENNA = wms::tile_id(48.2082f, 16.3719f, 9);

Game::Game(size_t width, size_t height)
    : Window(width, height), m_terrain_renderer(INNSBRUCK, 4, {glm::vec2(-500.0f), glm::vec2(500.0f)})
{
  SDL_ShowCursor(SDL_FALSE);
  SDL_CaptureMouse(SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  float fov = 45.0f;
  float aspect_ratio = float(width) / float(height);
  float near = 1.0f, far = 1000.0f;
  m_camera.set_projection_matrix(glm::perspective(glm::radians(fov), aspect_ratio, near, far));
  m_camera.set_local_position(glm::vec3(0.0f, 40.f, 0.0f));
}

void Game::render(float dt)
{
  glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto camera_position = m_camera.get_local_position();
  auto lod_focus = glm::vec2(camera_position.x, camera_position.z);

#if INTERSECT_PLANE
  auto camera_direction = m_camera.transform_direction(glm::vec3(0.0f, 0.0f, -1.0f));
  auto camera_target = camera_position + camera_direction * 1000.0f;

  float t;
  glm::vec3 point;
  Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 20.0f, 0.0f));
  Segment segment(camera_position, camera_target);

  if (segment_vs_plane(segment, plane, t)) {
    glm::vec3 point = segment.point_at(t);
    auto clamped_point = clamp(glm::vec2(point.x, point.z), m_terrain_renderer.bounds());
    lod_focus = glm::mix(lod_focus, clamped_point, 0.5);
  }
#endif

  m_terrain_renderer.render(m_camera, lod_focus);

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
        switch (sdl_event.key.keysym.sym) {
          case SDLK_n:
            m_terrain_renderer.set_zoom_levels(m_terrain_renderer.zoom_levels() + 1);
            break;
          case SDLK_m:
            m_terrain_renderer.set_zoom_levels(m_terrain_renderer.zoom_levels() - 1);
            break;
        }
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

  float speed = 100.0f;

  if (key_states[SDL_SCANCODE_W]) {
    position -= forward * speed * dt;
  }
  if (key_states[SDL_SCANCODE_A]) {
    position -= right * speed * dt;
  }
  if (key_states[SDL_SCANCODE_S]) {
    position += forward * speed * dt;
  }
  if (key_states[SDL_SCANCODE_D]) {
    position += right * speed * dt;
  }

  m_camera.set_local_position(position);
}

void Game::update(float dt) {}
