#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "Common.h"
#include "TerrainRenderer.h"
#include "TileUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

constexpr auto range_0_1 = Bounds<glm::vec2>(glm::vec2(0.0f), glm::vec2(1.0f));

TEST_CASE("test lod downsampling")
{
  auto root_tile_6 = TileId(6U, 34U, 22U);
  auto root_tile_7 = TileId(7U, 68U, 44U);

  SECTION("level 2")
  {
    auto root_tile = TileId(7U, 68U, 44U);
    auto child_0 = root_tile.children()[0];

    auto uv_0 = rescale_uv(root_tile, child_0);
    std::cout << uv_0 << std::endl;

    auto grand_children = child_0.children();

    auto uv_1 = rescale_uv(root_tile, grand_children[0]);
    std::cout << uv_1  << std::endl;

  }



  SECTION("6")
  {
    auto tile = TileId(12U, 2177U, 1436U);
    auto uv = rescale_uv(root_tile_6, tile);
    std::cout << uv << std::endl;
  }

  SECTION("7")
  {
    auto tile = TileId(12U, 2177U, 1436U);
    auto uv = rescale_uv(root_tile_7, tile);
    std::cout << uv << std::endl;
  }

  SECTION("tmp")
  {

std::vector tiles = {
TileId(12U, 2176U, 1437),
TileId(12U, 2178U, 1436),
TileId(12U, 2178U, 1435),
TileId(12U, 2177U, 1436),
TileId(12U, 2176U, 1436),
TileId(12U, 2177U, 1435),
TileId(12U, 2176U, 1435),
};


  for (auto& tile : tiles) {
    std::cout << tile << ": " << rescale_uv(root_tile_6, tile) << std::endl;
  }


  }
}
