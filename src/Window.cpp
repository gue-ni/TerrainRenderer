#include "Window.h"

Window::Window(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_width), static_cast<int>(m_height),
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    m_context = SDL_GL_CreateContext(m_window);

    glewExperimental = GL_TRUE;
    glewInit();
}

Window::~Window()
{
    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::run()
{
    while (!m_quit) {
        read_input();
        render();
    }
}

void Window::read_input()
{
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event) != 0) {
        if (sdl_event.type == SDL_QUIT) {
            m_quit = true;
            break;
        }

        if (sdl_event.type == SDL_KEYDOWN) {
            SDL_Keycode key = sdl_event.key.keysym.sym;

            if (key == SDL_SCANCODE_ESCAPE) {

                m_quit = true;
            }
        }
    }
}

void Window::render()
{
    glViewport(0, 0, static_cast<GLsizei>(m_width),
        static_cast<GLsizei>(m_height));
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(m_window);
}
