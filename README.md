# Terrain Renderer

![cmake workflow badge](https://github.com/gue-ni/TerrainRenderer/actions/workflows/cmake.yml/badge.svg)

A simple OpenGL Terrain Renderer. Orthophoto data is provided by [basemap.at](https://basemap.at/), elevation data is from
[geoland.at](http://www.geoland.at/). This data is limited to Austria and as such only Austria is rendered.

![](assets/Screenshot_2024-02-16_203330.png)

## Features

- [x] Real-world elevation and orthofoto data
- [x] Multithreaded terrain tile loading
- [X] Quadtree level-of-detail
- [X] Frustum Culling
- [ ] Realistic Atmosphere with Rayleigh scattering

## Build Instructions

This project depends on [SDL2](https://www.libsdl.org/), [GLM](https://github.com/g-truc/glm), 
[GLEW](https://glew.sourceforge.net/) and [libcpr](https://github.com/libcpr/cpr). You should be able to 
build by following the steps in the [cmake.yml](./.github/workflows/cmake.yml).

```
git clone --recursive git@github.com:gue-ni/TerrainRenderer.git
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build --config Release --parallel
```
