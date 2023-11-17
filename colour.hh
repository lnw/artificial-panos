#pragma once

#include <cstdint>

struct colour {
  constexpr colour() = default;
  constexpr colour(uint8_t r, uint8_t g, uint8_t b): r_(r), g_(g), b_(b) {}
  uint8_t r_ = 0, g_ = 0, b_ = 0;

  operator int32_t() const {
    return 127 << 24 | r_ << 16 | g_ << 8 | b_;
  }
};
