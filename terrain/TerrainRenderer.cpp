#include "TerrainRenderer.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>

#include "Collision.h"
#include "Common.h"

#define ENABLE_FOG      1
#define ENABLE_FALLBACK 1
#define ENABLE_SKYBOX   1

/* clang-format off */
const char* shader_vert =
#include "generated/terrain.vert"
;

const char* shader_frag =
#include "generated/terrain.frag"
;

const char* skybox_vert =
#include "generated/sky.vert"
;

const char* skybox_frag =
#include "generated/sky.frag"
;
/* clang-format on */

static AABB aabb_from_node(const Node* node)
{
  float height = 100.0f;  // TODO: do something smarter
  return AABB({node->min.x, 0.0f, node->min.y}, {node->max.x, height, node->max.y});
}

static Bounds<glm::vec2> rescale_uv(const TileId& parent_tile_id, const TileId& tile_id)
{
  // what is the problem here?
  unsigned zoom_delta = tile_id.zoom - parent_tile_id.zoom;
  unsigned num_tiles = 1 << zoom_delta;

  // maybe it is here
  auto scaled_root_tile = TileId(tile_id.zoom, parent_tile_id.x * num_tiles, parent_tile_id.y * num_tiles);

  unsigned delta_x = tile_id.x - scaled_root_tile.x;
  unsigned delta_y = tile_id.y - scaled_root_tile.y;

  float factor = 1.0f / num_tiles;

  Bounds<glm::vec2> uv = {glm::vec2((delta_x + 0) * factor, (delta_y + 0) * factor),
                          glm::vec2((delta_x + 1) * factor, (delta_y + 1) * factor)};

  return uv;
}

TerrainRenderer::TerrainRenderer(const TileId& root_tile, unsigned max_zoom_level_range,
                                 const Bounds<glm::vec2>& bounds)
    :
#if NDEBUG
      m_terrain_shader(std::make_unique<ShaderProgram>(shader_vert, shader_frag)),
      m_sky_shader(std::make_unique<ShaderProgram>(skybox_vert, skybox_frag)),
#else
      m_terrain_shader(ShaderProgram::create_from_files(
          "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/terrain.vert",
          "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/terrain.frag")),
      m_sky_shader(ShaderProgram::create_from_files(
          "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/sky.vert",
          "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/sky.frag")),
#endif
      m_root_tile(root_tile),
      m_chunk(32, 1.0f),
      m_bounds(bounds),
      m_coord_bounds(root_tile.bounds()),
      m_max_zoom_level_range(max_zoom_level_range),
      min_zoom(root_tile.zoom),
      max_zoom(root_tile.zoom + max_zoom_level_range)
{
  // the rendered terrain does not necessarily match with it's size in meters
  float width = m_bounds.size().x;
  float tile_width = m_root_tile.width_in_meters();
  m_terrain_scaling_factor = width / tile_width;

// for decoding the height map
#if 0
  const float min_elevation = 0.0f, max_elevation = 8191.0f;
#else
  const float min_elevation = 0.0f, max_elevation = 3795.0f;
#endif
  m_height_scaling_factor = (max_elevation - min_elevation);

#if 1
  (void)m_tile_cache.tile_texture_sync(m_root_tile, TileType::ORTHO);
  (void)m_tile_cache.tile_texture_sync(m_root_tile, TileType::HEIGHT);

  // request low zoom tiles as fallback
  for (auto& child : m_root_tile.children()) {
    (void)m_tile_cache.tile_texture_sync(child, TileType::ORTHO);
    (void)m_tile_cache.tile_texture_sync(child, TileType::HEIGHT);
  }
#endif
}

void TerrainRenderer::reload_shaders()
{
#if !NDEBUG
  ShaderProgram::reload_from_files(std::move(m_terrain_shader),
                                   "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/terrain.vert",
                                   "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/terrain.frag");

  ShaderProgram::reload_from_files(std::move(m_sky_shader),
                                   "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/sky.vert",
                                   "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/sky.frag");
#endif
}

float TerrainRenderer::elevation(const glm::vec2& point)
{
  Coordinate coord = point_to_coordinate(point);
  return m_tile_cache.elevation(coord) * m_height_scaling_factor;
}

float TerrainRenderer::altitude_over_terrain(const glm::vec2& point, float altitude)
{
  float altitude_in_meters = altitude / scaling_factor();
  float elevation = this->elevation(point);
  return std::max(0.0f, altitude_in_meters - elevation);
}

Plane TerrainRenderer::collider(const glm::vec2& point)
{
  float elevation = this->elevation(point);
  glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);  // TODO: fix this
  glm::vec3 point_on_plane = glm::vec3(point.x, elevation * scaling_factor(), point.y);
  return Plane(normal, point_on_plane);
}

Coordinate TerrainRenderer::point_to_coordinate(const glm::vec2& point) const
{
  auto coord_min = m_coord_bounds.min.to_vec2();
  auto coord_max = m_coord_bounds.max.to_vec2();
  return map_range(point, m_bounds.min, m_bounds.max, coord_min, coord_max);
}

glm::vec2 TerrainRenderer::coordinate_to_point(const Coordinate& coord) const
{
  auto coord_min = m_coord_bounds.min.to_vec2();
  auto coord_max = m_coord_bounds.max.to_vec2();
  return map_range(coord.to_vec2(), coord_min, coord_max, m_bounds.min, m_bounds.max);
}

void TerrainRenderer::calculate_zoom_levels(const glm::vec2& center, float altitude)
{
  // linear interpolate
  // this should probably be something more clever
  float alt = altitude_over_terrain(center, altitude);
  float min_alt = 0, max_alt = 25000;
  float normalized_height = alt / (max_alt - min_alt);
  float factor = glm::clamp(1.0f - normalized_height, 0.0f, 1.0f);

  max_zoom = std::max(int(TileId::MAX_ZOOM * factor), int(m_root_tile.zoom + 1U));

  int requested_zoom_range = max_zoom - m_root_tile.zoom;
  int zoom_range = glm::clamp(requested_zoom_range, 1, m_max_zoom_level_range);

  min_zoom = max_zoom - zoom_range;
}

glm::vec2 TerrainRenderer::calculate_lod_center(const Camera& camera)
{
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 position3 = camera.world_position();
  glm::vec2 position = {position3.x, position3.z};
  glm::vec3 forward = -camera.local_z_axis();

  if (intersect_terrain) {
    // the terrain we are looking at should be the highest lod

    float t;
    Ray ray(position3, forward);
    Plane plane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 30.0f, 0.0f));

    if (ray_vs_plane(ray, plane, t)) {
      glm::vec3 point = ray.point_at(t);
      glm::vec2 new_position = {point.x, point.z};
      return clamp_range(new_position, m_bounds);
    } else {
      return position;
    }
  } else {
    // we don't want the lod center the be right below the camera. Depending
    // on the altitude it should be in front of the camera.
    glm::vec2 view_direction = glm::normalize(glm::vec2(forward.x, forward.z));

    float alt = position3.y / scaling_factor();
    float min_alt = 0, max_alt = 20000;

    float distance_to_horizon = map_range(alt, min_alt, max_alt, 0.0f, max_horizon);

    glm::vec2 horizon = position + view_direction * distance_to_horizon;
    horizon = clamp_range(horizon, m_bounds);

    // if the camera is looking down, the lod center should be directly below the camera
    float t = glm::max(glm::dot(forward, -up), 0.0f);

    return glm::mix(horizon, position, t);
  }
}

Texture* TerrainRenderer::find_cached_lower_zoom_parent(Node* node, Bounds<glm::vec2>& uv, const TileType& type,
                                                        TileId& used)
{
  Texture* parent_texture = nullptr;
  TileId tile_id = node->id;
  used = tile_id;

  unsigned zoom = m_root_tile.zoom + node->depth;

  Node* parent = node->parent;
  TileId parent_tile_id = tile_id;

#if 1
  while (parent != nullptr) {
    parent_tile_id = parent->id;
    parent_texture = m_tile_cache.tile_texture_cached(parent_tile_id, type);
    if (parent_texture) break;
    parent = parent->parent;
  }
#endif

#if 1
  if (!parent_texture) {
    parent_tile_id = m_root_tile;
    parent_texture = m_tile_cache.tile_texture(m_root_tile, type);
    // assert(parent_texture);
  }
#endif

  uv = rescale_uv(parent_tile_id, tile_id);

  used = parent_tile_id;

  return parent_texture;
}

void TerrainRenderer::render(const Camera& camera)
{
  glm::vec3 position = camera.world_position();
  glm::vec2 center = glm::vec2(position.x, position.z);
  float altitude = position.y;

  Coordinate coord = point_to_coordinate(center);

  glm::vec3 lat_lon_alt = glm::vec3(coord.lat, altitude / m_terrain_scaling_factor, coord.lon);

  // focus point for the level of detail
  glm::vec2 lod_center = center;

  if (smart_lod) {
    lod_center = calculate_lod_center(camera);
  }

  lod_center = clamp_range(lod_center, m_bounds);

  if (!manual_zoom) {
    calculate_zoom_levels(center, altitude);
  }

  QuadTree quad_tree(lod_center, m_bounds.min, m_bounds.max, max_zoom - m_root_tile.zoom, m_root_tile);

  if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  m_terrain_shader->bind();
  m_terrain_shader->set_uniform("u_view", camera.view_matrix());
  m_terrain_shader->set_uniform("u_proj", camera.projection_matrix());
  m_terrain_shader->set_uniform("u_camera_position", camera.world_position());
  m_terrain_shader->set_uniform("u_height_scaling_factor", m_height_scaling_factor);
  m_terrain_shader->set_uniform("u_terrain_scaling_factor", m_terrain_scaling_factor);

  m_terrain_shader->set_uniform("u_debug_view", debug_view);
  m_terrain_shader->set_uniform("u_shading", shading);

  const glm::vec3 sky_color_1 = gfx::rgb(0xB8DEFD);
  const glm::vec3 sun_color = glm::vec3(1.0, 0.9, 0.7);

  const glm::vec3 sun_direction = direction_from_spherical(glm::radians(sun_elevation), glm::radians(sun_azimuth));

  m_terrain_shader->set_uniform("u_sun_dir", sun_direction);
  m_terrain_shader->set_uniform("u_sun_color", sun_color);

  const glm::vec3 dark_blue = gfx::rgb(0x597AE8);
  const glm::vec3 light_blue = gfx::rgb(0xC7E8F7);

  m_terrain_shader->set_uniform("u_light_blue", light_blue);
  m_terrain_shader->set_uniform("u_dark_blue", dark_blue);

  m_terrain_shader->set_uniform("u_fog_near", 0.0f);
  m_terrain_shader->set_uniform("u_fog_far", fog_far);
  m_terrain_shader->set_uniform("u_fog_density", fog_density);

  // This render function is executed on all quadtree nodes.
  auto render_tile = [&, this](Node* node) {
    TileId tile_id = node->id;

    Texture *albedo = nullptr, *heightmap = nullptr;

#if 1
    albedo = m_tile_cache.tile_texture(tile_id, TileType::ORTHO);
    heightmap = m_tile_cache.tile_texture(tile_id, TileType::HEIGHT);
#endif

    // Render only part of tile if we fallback to lower resolution.
    Bounds<glm::vec2> albedo_uv(glm::vec2(0.0f), glm::vec2(1.0f));
    Bounds<glm::vec2> height_uv(glm::vec2(0.0f), glm::vec2(1.0f));

    TileId albedo_tile_id, height_tile_id;

#if ENABLE_FALLBACK
    if (!albedo) {
      albedo = find_cached_lower_zoom_parent(node, albedo_uv, TileType::ORTHO, albedo_tile_id);
    }

    if (!heightmap) {
      heightmap = find_cached_lower_zoom_parent(node, height_uv, TileType::HEIGHT, height_tile_id);
    }
#endif

    if (albedo && heightmap) {
      m_terrain_shader->set_uniform("u_zoom", node->depth);

      const float pixel_per_tile = 128;
      float tile_width = tile_id.width_in_meters();
      float pixel_resolution = tile_width / pixel_per_tile;
      m_terrain_shader->set_uniform("u_pixel_resolution", pixel_resolution);

      albedo->bind(0);
      m_terrain_shader->set_uniform("u_albedo_texture", 0);
      m_terrain_shader->set_uniform("u_albedo_uv_min", albedo_uv.min);
      m_terrain_shader->set_uniform("u_albedo_uv_max", albedo_uv.max);

      heightmap->bind(1);
      m_terrain_shader->set_uniform("u_height_texture", 1);
      m_terrain_shader->set_uniform("u_height_uv_min", height_uv.min);
      m_terrain_shader->set_uniform("u_height_uv_max", height_uv.max);

      m_chunk.draw(m_terrain_shader.get(), node->min, node->max);
    }
  };

  Frustum frustum(camera.view_projection_matrix());

  // frustum culling
  auto is_visible = [&](Node* node) {
    AABB aabb = aabb_from_node(node);
    return aabb_vs_frustum(aabb, frustum);
  };

#if 1
  // ideally we should test for visibility here. In a quadtree, if a parent node
  // is not visible, it's children will also not be visible.
  std::vector<Node*> nodes;

  std::function<void(Node*)> visitor = [&](Node* node) {
    if (node->is_leaf && min_zoom <= (m_root_tile.zoom + node->depth) && (!frustum_culling || is_visible(node))) {
      nodes.push_back(node);
    }
  };

  quad_tree.visit(visitor);
#else
  // only leaves are rendered
  auto nodes = quad_tree.leaves();
#endif

  // Sort nodes so biggest zoom level is rendered and requested first
  std::sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) { return a->depth > b->depth; });

  //  we should really exploit our quadtree stucture to reduce the frustum culling tests
  std::for_each(nodes.begin(), nodes.end(), render_tile);

#if ENABLE_SKYBOX
  if (!wireframe) {
    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);

    m_sky_shader->bind();
    m_sky_shader->set_uniform("u_view", glm::mat4(glm::mat3(camera.view_matrix())));
    m_sky_shader->set_uniform("u_proj", camera.projection_matrix());
    m_sky_shader->set_uniform("u_camera_position", camera.world_position());
    m_sky_shader->set_uniform("u_sky_color", sky_color_1);
    m_sky_shader->set_uniform("u_light_blue", light_blue);
    m_sky_shader->set_uniform("u_dark_blue", dark_blue);
    m_sky_shader->set_uniform("u_sun_dir", sun_direction);
    m_sky_shader->set_uniform("u_sun_color", sun_color);
    m_sky_shader->set_uniform("u_lat_lon_alt", lat_lon_alt);
    m_sky_box.draw(m_sky_shader.get(), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);

    glDepthFunc(GL_LESS);
    glCullFace(GL_FRONT);
  }
#endif

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
