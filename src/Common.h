#pragma once

using uint = unsigned int;

template <typename T>
inline T map_range(const T& s, const T& in_min, const T& in_max, const T& out_min, const T& out_max)
{
  return out_min + (s - in_min) * (out_max - out_min) / (in_max - in_min);
}
