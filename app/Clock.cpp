#include "Clock.h"

#include <SDL.h>

void Clock::init() { m_now = SDL_GetPerformanceCounter(); }

void Clock::tick()
{
  m_last = m_now;
  m_now = SDL_GetPerformanceCounter();
  uint64_t freqency = SDL_GetPerformanceFrequency();
  m_delta = static_cast<float>(m_now - m_last) / static_cast<float>(freqency);
  m_accumulator += m_delta;
}
