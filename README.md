# Terrain Renderer

![msbuild workflow badge](https://github.com/gue-ni/TerrainRenderer/actions/workflows/msbuild.yml/badge.svg)

A simple OpenGL Terrain Renderer.

![](assets/Image_2024-01-22_23-24-21.png)

## Features

- [x] Real-world height and texture data 
- [x] Multithreaded terrain tile loading
- [X] Quadtree level-of-detail 
- [ ] Frustum Culling

## Build Instructions

This project depends on SDL2, GLM, GLEW and libcpr. You should be able to build on
windows by following the steps in the [msbuild.yml](./.github/workflows/msbuild.yml).

