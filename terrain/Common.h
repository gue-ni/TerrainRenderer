#pragma once

#include <glm/glm.hpp>

template <typename T>
struct Bounds {
  T min, max;
  constexpr Bounds(const T& min_, const T& max_) : min(min_), max(max_) {}
  T size() const { return max - min; }
  T center() const { return min + size() / T(2); }
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const Bounds<T>& b)
{
  return os << b.min << ", " << b.max;
}

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

template <typename T>
inline T clamp_range(const T& value, const Bounds<T>& range)
{
  return max(range.min, min(value, range.max));
}

inline glm::vec3 direction_from_spherical(float pitch, float yaw)
{
  return {std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)};
}

template <typename glm_vec>
inline bool contains(const glm_vec& value, const Bounds<glm_vec>& bounds)
{
  return glm::all(glm::lessThanEqual(bounds.min, value)) && glm::all(glm::lessThanEqual(value, bounds.max));
}

#if 0
template <typename glm_vec>
inline bool contains(const Bounds<glm_vec2> & a, const Bounds<glm_vec>& b)
{
  return 
}
#endif
