#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>

#include "Common.h"
#include "TerrainRenderer.h"
#include "TileUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

TEST_CASE("TileService")
{
  const std::string cache = "tiles/ortho-cache-test";
  const std::string url = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile";
  const UrlPattern pattern = UrlPattern::ZYX_Y_SOUTH;

  try {
    std::filesystem::remove_all(cache);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  TileService tile_service(url, pattern, "", cache);

  std::vector tiles = {
      TileId(0u, 0u, 0u),
      TileId(1u, 0u, 0u),
      TileId(2u, 0u, 0u),
      TileId(3u, 0u, 0u),
  };

  for (auto& tile : tiles) {
    Image* image = tile_service.get_tile(tile);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  for (auto& tile : tiles) {
    Image* image = tile_service.get_tile_cached(tile);
    CHECK(image != nullptr);
  }
}
