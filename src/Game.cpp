#include "Game.h"

#include "Collision.h"

#define INTERSECT_PLANE 0

const Coordinate GROSS_GLOCKNER(47.0742f, 12.6947f);

const Coordinate SCHNEEBERG(47.7671f, 15.8056f);

const Coordinate INNSBRUCK(47.2692f, 11.4041f);

const Coordinate root = INNSBRUCK;

const unsigned zoom = 7;

const float terrain_width = wms::tile_width(root.lat, zoom) * 0.01f;

Game::Game(size_t width, size_t height)
    : Window(width, height),
      m_terrain_renderer(TileId(root, zoom), 4, {glm::vec2(-terrain_width / 2.0f), glm::vec2(terrain_width / 2.0f)})
{
  float fov = 45.0f, aspect_ratio = float(width) / float(height), near = 1.0f, far = 100000.0f;
  m_camera.set_projection_matrix(glm::radians(fov), aspect_ratio, near, far);
  m_camera.set_local_position(glm::vec3(0.0f, 5000.0f * m_terrain_renderer.scaling_factor(), 0.0f));
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
  glm::vec3 camera_position = m_camera.local_position();
  glm::vec2 center = {camera_position.x, camera_position.z};

  if (m_terrain_renderer.intersect_terrain) {
    glm::vec3 camera_direction = m_camera.transform_direction(glm::vec3(0.0f, 0.0f, -1.0f));

    float t;
    Ray ray{camera_position, camera_direction};
    Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 30.0f, 0.0f));

    if (ray_vs_plane(ray, plane, t)) {
      glm::vec3 point = ray.point_at(t);
      glm::vec2 clamped_point = clamp(glm::vec2(point.x, point.z), m_terrain_renderer.bounds());
      center = glm::mix(center, clamped_point, 0.5);
    }
  }

  m_terrain_renderer.render(m_camera, center, camera_position.y);
}

void Game::render_ui()
{
  ImGuiWindowFlags window_flags = 0;

  ImGui::SetNextWindowPos(ImVec2(10, 10));

  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
  ImGui::Begin("Config", nullptr, window_flags);

  glm::vec3 pos = m_camera.local_position();
  glm::vec2 pos2 = {pos.x, pos.z};

  Coordinate coord = m_terrain_renderer.point_coordinate(pos);

  glm::vec3 forward = -m_camera.local_z_axis();
  int angle = static_cast<int>(glm::degrees(std::atan2(forward.x, forward.z)));
  int heading = (360 + angle) % 360;

  ImGui::Text("Camera Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
  ImGui::Text("Lat: %.4f, Lon: %.4f", coord.lat, coord.lon);
  ImGui::Text("Heading %d", heading);
  ImGui::Text("Terrain Elevation: %.2f", m_terrain_renderer.terrain_elevation(pos2));
  ImGui::Text("Altitude over terrain: %.2f", m_terrain_renderer.altitude_over_terrain(pos2, pos.y));
  ImGui::Text("Zoom Levels: [%d, %d]", m_terrain_renderer.root_tile().zoom,
              m_terrain_renderer.root_tile().zoom + m_terrain_renderer.zoom_levels());
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
