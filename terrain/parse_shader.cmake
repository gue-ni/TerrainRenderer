function(make_includable input_file output_file)
    file(READ ${input_file} content)
    set(delim "")
    set(content "R\"${delim}(\n${content})${delim}\"")
    file(WRITE ${output_file} "${content}")
endfunction(make_includable)

make_includable(
  "${SHADER_DIR}/shaders/sky.frag" 
  "${SHADER_DIR}/generated/sky.frag")

make_includable(
  "${SHADER_DIR}/shaders/sky.vert" 
  "${SHADER_DIR}/generated/sky.vert")

make_includable(
  "${SHADER_DIR}/shaders/terrain.vert" 
  "${SHADER_DIR}/generated/terrain.vert")

make_includable(
  "${SHADER_DIR}/shaders/terrain.frag" 
  "${SHADER_DIR}/generated/terrain.frag")
