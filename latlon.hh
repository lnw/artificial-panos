#pragma once

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "degrad.hh"

enum class Unit { deg,
                  rad };


template <typename T, Unit UU>
class LatLon {
public:
  using value_type = T;

  constexpr LatLon() noexcept = default;
  constexpr LatLon(T lat, T lon) noexcept: dat_{lat, lon} {}
  explicit constexpr LatLon(std::pair<T, T> p) noexcept: dat_{p.first, p.second} {}

  template <typename V>
  constexpr LatLon(LatLon<V, UU> p) noexcept: dat_{static_cast<T>(p.lat()), static_cast<T>(p.lon())} {}

  constexpr auto unit() const noexcept { return UU; }
  constexpr auto lat() const noexcept { return dat_[0]; }
  constexpr auto lon() const noexcept { return dat_[1]; }

  constexpr auto begin() const noexcept { return dat_.begin(); }
  constexpr auto end() const noexcept { return dat_.end(); }

  template <typename V = void>
  requires(UU == Unit::rad)
  constexpr auto to_deg() const noexcept {
    return LatLon<T, Unit::deg>(lat(), lon()) * rad2deg;
  }

  template <typename V = void>
  requires(UU == Unit::deg)
  constexpr auto to_rad() const noexcept {
    return LatLon<T, Unit::rad>(lat(), lon()) * deg2rad;
  }

  template <typename V>
  requires(std::is_arithmetic_v<V>)
  constexpr LatLon& operator*=(V rhs) noexcept {
    for (auto& v : dat_) {
      v *= rhs;
    }
    return *this;
  }

  template <typename U>
  requires(std::is_arithmetic_v<U>)
  friend constexpr auto operator*(U lhs, const LatLon<T, UU>& rhs) noexcept {
    using V = typename std::common_type_t<T, U>;
    return LatLon<V, UU>(rhs) *= lhs;
  }

  template <typename U>
  requires(std::is_arithmetic_v<U>)
  friend constexpr auto operator*(const LatLon<T, UU>& lhs, U rhs) noexcept {
    using V = typename std::common_type_t<T, U>;
    return LatLon<V, UU>(lhs) *= rhs;
  }


  template <std::size_t Index>
  requires(Index < 2)
  constexpr std::tuple_element_t<Index, LatLon> get() const noexcept {
    return dat_[Index];
  }

  template <std::size_t Index>
  requires(Index < 2)
  constexpr std::tuple_element_t<Index, LatLon>& get() noexcept {
    return dat_[Index];
  }

  // constexpr bool operator<=>(const LatLon&  ) const = default;
  constexpr bool operator==(const LatLon&) const noexcept = default;
  constexpr std::strong_ordering operator<=>(const LatLon& ll) const noexcept {
    if (lat() < ll.lat())
      return std::strong_ordering::less;
    if (lat() > ll.lat())
      return std::strong_ordering::greater;
    if (lon() < ll.lon())
      return std::strong_ordering::less;
    if (lon() > ll.lon())
      return std::strong_ordering::greater;
    return std::strong_ordering::equivalent;
  }

  friend std::ostream& operator<<(std::ostream& S, const LatLon& ll) {
    S << "{" << ll.lat() << ", " << ll.lon() << "}";
    return S;
  }

private:
  std::array<T, 2> dat_{};
};

namespace std {
template <typename T, Unit UU>
struct tuple_size<::LatLon<T, UU>> {
  static constexpr size_t value = 2;
};
template <typename T, Unit UU, uint64_t Index>
struct tuple_element<Index, ::LatLon<T, UU>> {
  using type = T;
};
} // namespace std

template <typename T, Unit UU>
constexpr LatLon<T, UU> floor(LatLon<T, UU> ll) noexcept {
  return {std::floor(ll.lat()), std::floor(ll.lon())};
}

// one helper for pybind11
inline auto vll2vp(const std::vector<LatLon<int64_t, Unit::deg>>& vll) {
  std::vector<std::pair<int64_t, int64_t>> res(vll.size());
  for (int64_t i = 0; i < std::ssize(vll); i++) {
    res[i] = {vll[i].lat(), vll[i].lon()};
  }
  return res;
}
