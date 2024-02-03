#include "TerrainRenderer.h"

#include <cassert>
#include <chrono>
#include <iostream>

#include "Common.h"

#define ENABLE_FOG      1
#define ENABLE_FALLBACK 1
#define ENABLE_SKYBOX   1

const std::string shader_vert = R"(
#version 430
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_height_scaling_factor;
uniform vec2 u_height_uv_min;
uniform vec2 u_height_uv_max;
uniform sampler2D u_height_texture;
uniform uint u_zoom;

out vec2 uv;
out vec4 world_pos;

float altitude_from_color(vec4 color) { 
  return (color.r + color.g / 255.0); 
}

vec2 map_range(vec2 value, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) { 
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min); 
}

void main() {
  uv = a_tex;

  world_pos = u_model * vec4(a_pos, 1.0);

  vec2 scaled_uv = map_range(uv, vec2(0), vec2(1), u_height_uv_min, u_height_uv_max);

  vec4 height_sample = texture(u_height_texture, scaled_uv);

  float height = altitude_from_color(height_sample);

  world_pos.y = height * u_height_scaling_factor;

  // skirts on tiles
  if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
    world_pos.y = -2.0;
  }

  gl_Position = u_proj * u_view * world_pos;
}
)";

const std::string shader_frag = R"(
#version 430

in vec2 uv;
in vec4 world_pos;

out vec4 frag_color;

uniform vec2 u_albedo_uv_min;
uniform vec2 u_albedo_uv_max;
uniform sampler2D u_albedo_texture;
uniform vec3 u_fog_color;
uniform float u_fog_near;
uniform float u_fog_far;
uniform float u_fog_density;
uniform vec3 u_camera_position;
uniform uint u_zoom;

vec2 map_range(vec2 value, vec2 in_min, vec2 in_max, vec2 out_min, vec2 out_max) { 
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min); 
}

void main() {
  vec2 scaled_uv = map_range(uv, vec2(0), vec2(1), u_albedo_uv_min, u_albedo_uv_max);

  vec3 color = texture(u_albedo_texture, scaled_uv).rgb;

  if (u_fog_color != vec3(0)) {
    vec3 camera_dir = normalize(u_camera_position - world_pos.xyz);
    float camera_dist = length(world_pos.xyz - u_camera_position);
    float dist_ratio = 4.0 * camera_dist / u_fog_far;
    float fog_factor = 1.0 - exp(-dist_ratio * u_fog_density);

    vec3 sun_dir = normalize(vec3(0, 1, 2));
    vec3 sun_color = vec3(1.0, 0.9, 0.7);
    float sun_factor = max(dot(camera_dir, sun_dir), 0.0);

    //vec3 fog_color  = mix(u_fog_color, sun_color, pow(sun_factor, 8.0));
    vec3 fog_color = u_fog_color;

    color = mix(color, fog_color, fog_factor);
  }

  frag_color = vec4(color, 1);
}
)";

const std::string skybox_vert = R"(
#version 430
layout (location = 0) in vec3 a_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 uv;

void main() {
  uv = a_pos;
  vec4 pos = u_proj * u_view * vec4(a_pos, 1.0);
  gl_Position = pos.xyww;
}
)";

const std::string skybox_frag = R"(
#version 430

uniform vec3 u_sky_color_1;
uniform vec3 u_sky_color_2;

out vec4 frag_color;
in vec3 uv;

#define sq(x) (x * x)

vec3 map_cube_to_sphere(vec3 point_on_cube) {
    float x = point_on_cube.x, y = point_on_cube.y, z = point_on_cube.z;

    vec3 point_on_sphere;
    point_on_sphere.x = x * sqrt(1.0f - (sq(y) / 2.0f) - (sq(z) / 2.0f) + ((sq(y) * sq(z)) / 3.0f));
    point_on_sphere.y = y * sqrt(1.0f - (sq(z) / 2.0f) - (sq(x) / 2.0f) + ((sq(z) * sq(x)) / 3.0f));
    point_on_sphere.z = z * sqrt(1.0f - (sq(x) / 2.0f) - (sq(y) / 2.0f) + ((sq(x) * sq(y)) / 3.0f));
    return point_on_sphere;
}

void main() {
  vec3 spherical = map_cube_to_sphere(uv);

  vec3 color = mix(u_sky_color_1, u_sky_color_2, spherical.y);

  vec3 sun_dir = normalize(vec3(0, 1, 2));
  vec3 sun_color = vec3(1.0, 0.9, 0.7);
  float sun_factor = max(dot(spherical, sun_dir), 0.0);

  //color  = mix(color, sun_color, pow(sun_factor, 8.0));

  frag_color = vec4(color, 1);
}
)";

TerrainRenderer::TerrainRenderer(const TileId& root_tile, unsigned num_zoom_levels, const Bounds<glm::vec2>& bounds)
    : m_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_sky_shader(std::make_unique<ShaderProgram>(skybox_vert, skybox_frag)),
      m_root_tile(root_tile),
      m_chunk(32, 1.0f),
      m_bounds(bounds),
      m_tile_cache(m_root_tile, m_root_tile.zoom + num_zoom_levels),
      zoom_levels(num_zoom_levels)
{
  const float min_elevation = 0.0f, max_elevation = 8191.0f;

  float width = m_bounds.size().x;

  float tile_width = wms::tile_width(wms::tiley2lat(m_root_tile.y, m_root_tile.zoom), m_root_tile.zoom);

  m_terrain_scaling_factor = width / tile_width;

  m_height_scaling_factor = (max_elevation - min_elevation);

  // request low zoom tiles as fallback
  for (auto& child : m_root_tile.children()) {
    (void)m_tile_cache.tile_texture_sync(child, TileType::ORTHO);
    (void)m_tile_cache.tile_texture_sync(child, TileType::HEIGHT);
  }
}

void TerrainRenderer::render(const Camera& camera, const glm::vec2& center, float altitude)
{
  const auto terrain_center = clamp(center, m_bounds);

  int scaled_zoom_levels = 0;

  float elevation = terrain_elevation(center) * m_terrain_scaling_factor;

#if 1

  float root_width = m_bounds.size().x;

  float altitude_over_terrain = glm::max(0.0f, altitude - elevation);

  for (scaled_zoom_levels = 0; scaled_zoom_levels <= m_max_zoom_levels; scaled_zoom_levels++) {
    float width = root_width / (1 << scaled_zoom_levels);

    float tmp = width * 0.1f;

    if (tmp < altitude_over_terrain) break;
  }

#endif

  zoom_levels = 1 + scaled_zoom_levels;

  QuadTree quad_tree(m_bounds.min, m_bounds.max, zoom_levels);
  quad_tree.insert(terrain_center);

  if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_shader->bind();
  m_shader->set_uniform("u_view", camera.view_matrix());
  m_shader->set_uniform("u_proj", camera.projection_matrix());
  m_shader->set_uniform("u_camera_position", camera.world_position());
  m_shader->set_uniform("u_height_scaling_factor", m_height_scaling_factor * m_terrain_scaling_factor);

  const glm::vec3 sky_color_1 = gfx::rgb(0xB8DEFD);
  const glm::vec3 sky_color_2 = gfx::rgb(0x6F93F2);

#if ENABLE_FOG
  m_shader->set_uniform("u_fog_color", sky_color_1);
  m_shader->set_uniform("u_fog_near", 0.0f);
  m_shader->set_uniform("u_fog_far", fog_far);
  m_shader->set_uniform("u_fog_density", fog_density);

  float sun_elevation = 25.61f;
  float sun_azimuth = 179.85f;
  glm::vec3 sun_direction = vector_from_spherical(glm::radians(sun_elevation), glm::radians(sun_azimuth));
  m_shader->set_uniform("u_sun_dir", sun_direction);
#else
  m_shader->set_uniform("u_fog_color", glm::vec3(0.0f));
#endif

  auto render_tile = [this](Node* node) {
    if (!node->is_leaf) return;

    TileId tile_id = tile_id_from_node(node);

    Texture* albedo = m_tile_cache.tile_texture(tile_id, TileType::ORTHO);
    Texture* heightmap = m_tile_cache.tile_texture(tile_id, TileType::HEIGHT);

    // Render only part of tile if we fallback to lower resolution
    Bounds<glm::vec2> albedo_uv = {glm::vec2(0.0f), glm::vec2(1.0f)};
    Bounds<glm::vec2> height_uv = {glm::vec2(0.0f), glm::vec2(1.0f)};

#if ENABLE_FALLBACK
    if (!albedo) {
      albedo = find_cached_lower_zoom_parent(node, albedo_uv, TileType::ORTHO);
    }

    if (!heightmap) {
      heightmap = find_cached_lower_zoom_parent(node, height_uv, TileType::HEIGHT);
    }
#endif

    if (albedo && heightmap) {
      m_shader->set_uniform("u_zoom", tile_id.zoom);

      albedo->bind(0);
      m_shader->set_uniform("u_albedo_texture", 0);
      m_shader->set_uniform("u_albedo_uv_min", albedo_uv.min);
      m_shader->set_uniform("u_albedo_uv_max", albedo_uv.max);

      heightmap->bind(1);
      m_shader->set_uniform("u_height_texture", 1);
      m_shader->set_uniform("u_height_uv_min", height_uv.min);
      m_shader->set_uniform("u_height_uv_max", height_uv.max);

      m_chunk.draw(m_shader.get(), node->min, node->max);
    }
  };

  auto nodes = quad_tree.nodes();
  std::sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) { return a->depth < b->depth; });
  std::for_each(nodes.begin(), nodes.end(), render_tile);

#if ENABLE_SKYBOX

  glCullFace(GL_BACK);
  glDepthFunc(GL_LEQUAL);

  m_sky_shader->bind();
  m_sky_shader->set_uniform("u_view", glm::mat4(glm::mat3(camera.view_matrix())));
  m_sky_shader->set_uniform("u_proj", camera.projection_matrix());
  m_sky_shader->set_uniform("u_sky_color_1", sky_color_1);
  m_sky_shader->set_uniform("u_sky_color_2", sky_color_2);
  m_sky_box.draw(m_sky_shader.get(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);

  glDepthFunc(GL_LESS);
  glCullFace(GL_FRONT);
#endif

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float TerrainRenderer::terrain_elevation(const glm::vec2& point)
{
  Coordinate coord = point_coordinate(point);
  return m_tile_cache.terrain_elevation(coord) * m_height_scaling_factor;
}

float TerrainRenderer::altitude_over_terrain(const glm::vec2& point, float altitude)
{
  float altitude_in_meters = altitude / scaling_factor();
  float elevation = terrain_elevation(point);
  return std::max(0.0f, altitude_in_meters - elevation);
}

glm::vec2 TerrainRenderer::map_to_0_1(const glm::vec2& point) const
{
  assert(contains(point, m_bounds));
  return map_range(point, m_bounds, Bounds(glm::vec2(0.0f), glm::vec2(1.0f)));
}

Coordinate TerrainRenderer::point_coordinate(const glm::vec2& point) const
{
  return m_tile_cache.lat_lon(map_to_0_1(point));
}

TileId TerrainRenderer::tile_id_from_node(Node* node) const
{
  Coordinate coord = m_tile_cache.lat_lon(map_to_0_1(node->center()));
  return m_tile_cache.tile_id(coord, node->depth);
}

Texture* TerrainRenderer::find_cached_lower_zoom_parent(Node* node, Bounds<glm::vec2>& uv, const TileType& type)
{
  Texture* parent_texture = nullptr;
  TileId tile_id = tile_id_from_node(node);

  Node* parent = node->parent;
  TileId parent_tile_id;

  while (parent != nullptr) {
    parent_tile_id = tile_id_from_node(parent);
    parent_texture = m_tile_cache.tile_texture_cached(parent_tile_id, type);
    if (parent_texture) break;
    parent = parent->parent;
  }

  if (!parent_texture) {
    parent_tile_id = m_root_tile;
    parent_texture = m_tile_cache.tile_texture_cached(m_root_tile, type);
  }

  assert(parent_texture);

  unsigned zoom_delta = tile_id.zoom - parent_tile_id.zoom;
  unsigned num_tiles = 1 << zoom_delta;
  TileId scaled_root_tile(tile_id.zoom, parent_tile_id.x * num_tiles, parent_tile_id.y * num_tiles);
  unsigned delta_x = tile_id.x - scaled_root_tile.x;
  unsigned delta_y = tile_id.y - scaled_root_tile.y;
  float factor = 1.0f / num_tiles;

  uv.min = glm::vec2((delta_x + 0) * factor, (delta_y + 0) * factor);
  uv.max = glm::vec2((delta_x + 1) * factor, (delta_y + 1) * factor);

  return parent_texture;
}
