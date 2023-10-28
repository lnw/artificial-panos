#pragma once

#include <numbers>

template <typename T>
constexpr T deg2rad_v = std::numbers::pi_v<T> / T(180);
template <typename T>
constexpr T rad2deg_v = T(180) / std::numbers::pi_v<T>;

constexpr double deg2rad = deg2rad_v<double>;
constexpr double rad2deg = rad2deg_v<double>;

enum class Unit { deg,
                  rad };
