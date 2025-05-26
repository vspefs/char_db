//  char_db, the general encoding/decoding C++ library
//  Copyright (C) 2025 vspefs<vspefs@protonmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <array>
#include <ranges>

namespace char_db {

template <typename T>
  concept minimal_database_interface = requires (std::vector<typename T::char_type> seq)
  {
    typename T::char_type;
    { T::front_mblen (seq) } -> std::same_as<std::size_t>;
    { T::to_code_point (seq) } -> std::same_as<char32_t>;
    { T::template code_point_to<std::vector<typename T::char_type>> (std::declval<char32_t> ()) }
      -> std::same_as<std::vector<typename T::char_type>>;
  };

template <typename T, typename CharT>
  concept database_of =
    minimal_database_interface<T> && requires (std::vector<CharT> seq)
    {
      requires std::same_as<typename T::char_type, CharT>;

      { T::is_valid_char (seq) } -> std::same_as<bool>;

      { T::char_size (seq) } -> std::same_as<std::size_t>;
      { T::starts_with_valid_char (seq) } -> std::same_as<bool>;
      { T::validate_char_sequence (seq) } -> std::same_as<bool>;
    };

template <typename D, typename CharT>
  class database_interface
  {
  public:
    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::ranges::range_value_t<R>>
      static constexpr bool is_valid_char (R &&);

    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::ranges::range_value_t<R>>
      static constexpr std::size_t char_size (R &&);

    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::ranges::range_value_t<R>>
      static constexpr bool starts_with_valid_char (R &&);

    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::ranges::range_value_t<R>>
      static constexpr bool validate_char_sequence (R &&);
  };

class utf32 : public database_interface<utf32, char32_t>
{
public:
  using char_type = char32_t;

private:
  struct assigned_range_t
  {
    char32_t const start;
    char32_t const end;
  };

public:
  template <std::ranges::input_range R>
  requires std::same_as<char32_t, std::ranges::range_value_t<R>>
    static constexpr std::size_t front_mblen (R &&seq);

  template <std::ranges::input_range R>
  requires std::same_as<char32_t, std::ranges::range_value_t<R>>
    static constexpr char32_t to_code_point (R &&seq);

  template <std::ranges::range R>
  requires std::same_as<char32_t, std::ranges::range_value_t<R>>
    static constexpr R code_point_to (char32_t);

private:
  static constexpr auto assigned_ranges = std::to_array<assigned_range_t> ({
#include "generated/just_ranges.inc"
    });
};

class utf16 : public database_interface<utf16, char16_t>
{
public:
  using char_type = char16_t;

private:
  struct assigned_range_t
  {
    char32_t const start;
    char32_t const end;
  };

  struct surrogate_range_t
  {
    char16_t const start;
    char16_t const end;
  };

public:
  template <std::ranges::input_range R>
  requires std::same_as<char16_t, std::ranges::range_value_t<R>>
    static constexpr std::size_t front_mblen (R &&seq);

  template <std::ranges::input_range R>
  requires std::same_as<char16_t, std::ranges::range_value_t<R>>
    static constexpr char32_t to_code_point (R &&seq);

  template <std::ranges::range R>
  requires std::same_as<char16_t, std::ranges::range_value_t<R>>
    static constexpr R code_point_to (char32_t);

  static constexpr char32_t surrogate_pair_to_code_point (char16_t high, char16_t low) noexcept;

  static constexpr std::array<char16_t, 2> code_point_to_surrogate_pair (char32_t code_point);

  static constexpr bool is_high_surrogate (char16_t code_unit) noexcept;

  static constexpr bool is_low_surrogate (char16_t code_unit) noexcept;

  static constexpr bool is_bmp_code_point (char32_t code_point) noexcept;

  static constexpr bool is_non_bmp_code_point (char32_t  code_point) noexcept;

private:
  static constexpr auto bmp_assigned_ranges = std::to_array<assigned_range_t> ({
#include "generated/bmp_ranges.inc"
    });
  static constexpr auto non_bmp_assigned_ranges = std::to_array<assigned_range_t> ({
#include "generated/non_bmp_ranges.inc"
    });
  static constexpr auto high_surrogate_range = surrogate_range_t { 0xD800u, 0xDC00u };
  static constexpr auto low_surrogate_range = surrogate_range_t { 0xDC00u, 0xE000u };
};

class utf8 : public database_interface<utf8, char8_t>
{
public:
  using char_type = char8_t;
  static constexpr std::size_t from_continuation_byte = 0;

private:
  struct assigned_range_t
  {
    char32_t const start;
    char32_t const end;
  };

public:
  template <std::ranges::input_range R>
  requires std::same_as<char8_t, std::ranges::range_value_t<R>>
    static constexpr std::size_t front_mblen (R &&seq);

  template <std::ranges::input_range R>
  requires std::same_as<char8_t, std::ranges::range_value_t<R>>
    static constexpr char32_t to_code_point (R &&seq);

  template <std::ranges::range R>
  requires std::same_as<char8_t, std::ranges::range_value_t<R>>
    static constexpr R code_point_to (char32_t);

  static constexpr std::size_t trivial_mblen_from_unit (char8_t unit) noexcept;
  static constexpr char32_t extract_bits_from_code_unit (char8_t code_unit, std::size_t trivial_mblen) noexcept;
  static constexpr bool is_continuation_unit (char8_t code_unit) noexcept;

private:
  static constexpr auto assigned_ranges_1 = std::to_array<assigned_range_t> ({
#include "generated/utf8_ranges_1.inc"
    });
  static constexpr auto assigned_ranges_2 = std::to_array<assigned_range_t> ({
#include "generated/utf8_ranges_2.inc"
    });
  static constexpr auto assigned_ranges_3 = std::to_array<assigned_range_t> ({
#include "generated/utf8_ranges_3.inc"
    });
  static constexpr auto assigned_ranges_4 = std::to_array<assigned_range_t> ({
#include "generated/utf8_ranges_4.inc"
    });
};

} // namespace char_db

#include "database.tcc"