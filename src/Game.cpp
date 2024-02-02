#include "Game.h"

#include "Collision.h"

#define INTERSECT_PLANE 0

const TileId BLUDENZ = wms::tile_id(47.1599f, 9.8082f, 9);

const TileId GROSS_GLOCKNER = wms::tile_id(47.0742f, 12.6947f, 8);

const TileId SCHNEEBERG = wms::tile_id(47.7671f, 15.8056f, 9);

const TileId INNSBRUCK = wms::tile_id(47.2692f, 11.4041f, 8);

const TileId VIENNA = wms::tile_id(48.2082f, 16.3719f, 9);

const TileId ROOT = INNSBRUCK;

const float WIDTH = wms::tile_width(wms::tiley2lat(ROOT.y, ROOT.zoom), ROOT.zoom) * 0.01f;
// const float WIDTH = 5000.0f;

Game::Game(size_t width, size_t height)
    : Window(width, height), m_terrain_renderer(ROOT, 4, {glm::vec2(-WIDTH / 2.0f), glm::vec2(WIDTH / 2.0f)})
{
  // SDL_ShowCursor(SDL_FALSE);
  // SDL_CaptureMouse(SDL_TRUE);
  // SDL_SetRelativeMouseMode(SDL_TRUE);

  float fov = 45.0f, aspect_ratio = float(width) / float(height), near = 1.0f, far = 100000.0f;
  m_camera.set_projection_matrix(glm::radians(fov), aspect_ratio, near, far);
  m_camera.set_local_position(glm::vec3(0.0f, 40.f, 0.0f));
}

void Game::render(float dt)
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(m_window);
  ImGui::NewFrame();

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render_terrain();

  render_ui();

  SDL_GL_SwapWindow(m_window);
}

void Game::render_terrain()
{
  auto camera_position = m_camera.local_position();
  auto center = glm::vec2(camera_position.x, camera_position.z);

  if (m_terrain_renderer.intersect_terrain) {
    auto camera_direction = m_camera.transform_direction(glm::vec3(0.0f, 0.0f, -1.0f));
    auto camera_target = camera_position + camera_direction * 1000.0f;

    float t;
    Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 20.0f, 0.0f));
    Ray ray{camera_position, camera_direction};

    if (ray_vs_plane(ray, plane, t)) {
      glm::vec3 point = ray.point_at(t);
      auto clamped_point = clamp(glm::vec2(point.x, point.z), m_terrain_renderer.bounds());
      center = glm::mix(center, clamped_point, 0.5);
    }
  }

  m_terrain_renderer.render(m_camera, center);
}

void Game::render_ui()
{
  ImGuiWindowFlags window_flags = 0;

  ImGui::SetNextWindowPos(ImVec2(10, 10));

  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
  ImGui::Begin("Config", nullptr, window_flags);

  auto pos = m_camera.local_position();
  ImGui::Text("Camera Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
  ImGui::SliderInt("Zoom Levels", &m_terrain_renderer.zoom_levels, 1, 7);
  ImGui::Checkbox("Wireframe", &m_terrain_renderer.wireframe);
  ImGui::Checkbox("Ray Intersect", &m_terrain_renderer.intersect_terrain);
  ImGui::SliderFloat("Camera Speed", &m_speed, 50.0f, 5000.0f);
  ImGui::SliderFloat("Fog Far", &m_terrain_renderer.fog_far, 100.0f, 100000.0f);
  ImGui::SliderFloat("Fog Density", &m_terrain_renderer.fog_density, 0.0f, 10.0f);
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                                   event.window.windowID == SDL_GetWindowID(m_window))) {
      m_quit = true;
    }

    switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) m_mousedown = true;
        break;

      case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT) m_mousedown = false;
        break;

      case SDL_MOUSEMOTION: {
        bool hover =
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow);

        if (m_mousedown && !hover) {
          const float sensitivity = 0.20f;
          float delta_yaw = static_cast<float>(event.motion.xrel) * sensitivity;
          float delta_pitch = static_cast<float>(event.motion.yrel) * sensitivity;

          m_camera.yaw += delta_yaw;
          m_camera.pitch -= delta_pitch;
          m_camera.pitch = std::clamp(m_camera.pitch, -89.0f, 89.0f);

          glm::vec3 front = vector_from_spherical(glm::radians(m_camera.pitch), glm::radians(m_camera.yaw));

          glm::vec3 world_up = {0.0f, 1.0f, 0.0f};
          glm::vec3 right = glm::normalize(glm::cross(front, world_up));
          glm::vec3 up = glm::normalize(glm::cross(right, front));
          glm::vec3 position = m_camera.local_position();

          auto look_at = glm::lookAt(position, position + front, up);
          m_camera.set_local_transform(glm::inverse(look_at));
        }

        break;
      }
    }
  }

  const Uint8 *key_states = SDL_GetKeyboardState(nullptr);

  auto right = m_camera.local_x_axis();
  auto forward = m_camera.local_z_axis();
  auto position = m_camera.local_position();

  if (key_states[SDL_SCANCODE_W]) {
    position -= forward * m_speed * dt;
  }
  if (key_states[SDL_SCANCODE_A]) {
    position -= right * m_speed * dt;
  }
  if (key_states[SDL_SCANCODE_S]) {
    position += forward * m_speed * dt;
  }
  if (key_states[SDL_SCANCODE_D]) {
    position += right * m_speed * dt;
  }

  m_camera.set_local_position(position);
}

void Game::update(float dt) {}
