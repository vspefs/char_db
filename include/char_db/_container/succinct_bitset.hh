#pragma once

#include <cstdint>
#include <ranges>
#include <vector>
#include <ranges>
#include <span>

#include "../std_expo.hh"

namespace char_db::_container {

template <std::size_t N>
  class succinct_bitset
  {
  public:
    constexpr explicit succinct_bitset (std::from_range_t, std_expo::container_compatible_range<bool> auto &&);

    [[nodiscard]] consteval std::size_t size () const noexcept;
    [[nodiscard]] constexpr std::size_t count () const noexcept;
    [[nodiscard]] constexpr bool at (std::size_t) const noexcept;

    // Returns the number of bits equal to Value in [0, pos).
    template <bool Value>
      [[nodiscard]] constexpr std::size_t rank (std::size_t) const noexcept;

    // Returns the position of the (k+1)th bit equal to Value.
    template <bool Value>
      [[nodiscard]] constexpr std::size_t select (std::size_t) const noexcept;

  public:
    using uintword_t = std::uintptr_t;
    static constexpr std::size_t word_bit_size = CHAR_BIT * sizeof (uintword_t);
    static constexpr std::size_t l2_bit_size = word_bit_size;
    static constexpr std::size_t l1_bit_size = 64 * l2_bit_size;

    std::array<uintword_t, (N - 1) / word_bit_size + 1> bits_;
    std::array<std::size_t, (N - 1) / l1_bit_size + 1> l1_;
    std::array<std::uint16_t, (N - 1) / l2_bit_size + 1> l2_;
    std::size_t total_set_bits_ = 0;
  };

template <>
  class succinct_bitset<std::dynamic_extent>
  {
  public:
    constexpr explicit succinct_bitset (std::from_range_t, std_expo::container_compatible_range<bool> auto &&);

    [[nodiscard]] constexpr std::size_t size () const noexcept;
    [[nodiscard]] constexpr std::size_t count () const noexcept;
    [[nodiscard]] constexpr bool at (std::size_t) const noexcept;

    // Returns the number of bits equal to Value in [0, pos).
    template <bool Value>
      [[nodiscard]] constexpr std::size_t rank (std::size_t) const noexcept;

    // Returns the position of the (k+1)th bit equal to Value.
    template <bool Value>
      [[nodiscard]] constexpr std::size_t select (std::size_t) const noexcept;

  private:
    using uintword_t = std::uintptr_t;
    static constexpr std::size_t word_bit_size = CHAR_BIT * sizeof (uintword_t);
    static constexpr std::size_t l2_bit_size = word_bit_size;
    static constexpr std::size_t l1_bit_size = 64 * l2_bit_size;

    std::size_t total_bits_ = 0;
    std::vector<uintword_t> bits_;
    std::vector<std::size_t> l1_;
    std::vector<std::uint16_t> l2_;
    std::size_t total_set_bits_ = 0;
  };

} // namespace char_db::_container


#include "succinct_bitset.tcc"