#pragma once
#include <cstdint>

class Clock
{
 public:
  void init();
  void tick();
  float delta_time() const { return m_delta; }

 private:
  uint64_t m_last{0}, m_now{0};
  float m_delta{0.0f};
  float m_accumulator{0.0f};
};
