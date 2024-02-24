function(make_includable input_file output_file)
    message("hello")
    message("input_file = ${input_file}")
    message("output_file = ${output_file}")
    file(READ ${input_file} content)
    set(delim "")
    set(content "R\"${delim}(\n${content})${delim}\"")
    file(WRITE ${output_file} "${content}")
endfunction(make_includable)

#make_includable(shaders/terrain.frag generated/terrain.frag)
#make_includable(shaders/terrain.vert generated/terrain.vert)
#make_includable(shaders/sky.vert generated/sky.vert)

message("source dir: ${CMAKE_SOURCE_DIR}")
message("current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

make_includable(
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/sky.frag" 
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/generated/sky.frag")

make_includable(
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/sky.vert" 
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/generated/sky.vert")

make_includable(
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/terrain.vert" 
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/generated/terrain.vert")

make_includable(
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/shaders/terrain.frag" 
  "C:/Users/jakob/Documents/Projects/TerrainRenderer/terrain/generated/terrain.frag")



