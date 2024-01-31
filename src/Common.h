#pragma once

using uint = unsigned int;

template <typename T>
struct Bounds {
  T min, max;
  Bounds(const T& min_, const T& max_) : min(min_), max(max_) {}
  T size() const { return max - min; }
  T center() const { return min + size() / T(2); }
};

template <typename T>
inline T map_range(const T& value, const T& in_min, const T& in_max, const T& out_min, const T& out_max)
{
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

template <typename T>
inline T map_range(const T& value, const Bounds<T>& in, const Bounds<T>& out)
{
  return out.min + (value - in.min) * (out.max - out.min) / (in.max - in.min);
}
