#pragma once
#include "../gfx/gfx.h"
#include "Window.h"
#include <memory>
class Game : public Window {
public:
    Game(size_t width, size_t height);

private:
    std::unique_ptr<gfx::gl::ShaderProgram> m_shader { nullptr };
};
