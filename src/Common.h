#pragma once

using uint = unsigned int;

template <typename T>
inline T map_range(T s, T in_min, T in_max, T out_min, T out_max)
{
  return out_min + (s - in_min) * (out_max - out_min) / (in_max - in_min);
}
