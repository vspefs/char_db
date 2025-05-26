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

#include "database.hh"

namespace char_db {

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::ranges::range_value_t<R>>
    constexpr bool
    database_interface<D, CharT>::is_valid_char (R &&seq)
    {
      auto const mblen = D::front_mblen (seq);
      if (mblen != 0 && std::ranges::size (seq) == mblen)
        return true;
      else
        return false;
    }

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::ranges::range_value_t<R>>
    constexpr std::size_t
    database_interface<D, CharT>::char_size (R &&seq)
    {
      auto const sentinel = std::ranges::cend (seq);
      auto const begin = std::ranges::cbegin (seq);
      std::size_t size = 0;

      for (auto mblen = D::front_mblen (seq);
           0 != mblen;
           mblen = D::front_mblen (std::ranges::subrange (begin, sentinel)))
        {
          size++;
          std::ranges::advance (begin, mblen);
        }

      return size;
    }

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::ranges::range_value_t<R>>
    constexpr bool
    database_interface<D, CharT>::starts_with_valid_char (R &&seq)
    {
      return D::front_mblen (seq) != 0;
    }

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::ranges::range_value_t<R>>
    constexpr bool
    database_interface<D, CharT>::validate_char_sequence (R &&seq)
    {
      auto const sentinel = std::ranges::cend (seq);
      auto current = std::ranges::cbegin (seq);

      for (auto mblen = D::front_mblen (seq); 0 != mblen;
  	   mblen = D::front_mblen (std::ranges::subrange (current, sentinel)))
  	std::ranges::advance (current, mblen);

      return current == sentinel;
    }


template <std::ranges::input_range R>
requires std::same_as<char32_t, std::ranges::range_value_t<R>>
  constexpr std::size_t
  utf32::front_mblen (R &&seq)
  {
    if (std::ranges::empty (seq))
      return 0;

    auto const code_point = *std::ranges::cbegin (seq);

    std::size_t low = 0;
    std::size_t high = assigned_ranges.size () - 1;
    while (low <= high)
      if (std::size_t const mid = low + (high - low) / 2;
          code_point < assigned_ranges[mid].start)
        {
          if (mid == 0)
            return 0;
          high = mid - 1;
        }
      else if (assigned_ranges[mid].end <= code_point)
        low = mid + 1;
      else
        return 1;

    return 0;
  }

template <std::ranges::input_range R>
requires std::same_as<char32_t, std::ranges::range_value_t<R>>
  constexpr char32_t
  utf32::to_code_point (R &&seq)
  {
    return *std::ranges::cbegin (seq);
  }

template <std::ranges::range R>
requires std::same_as<char32_t, std::ranges::range_value_t<R>>
  constexpr R
  utf32::code_point_to (char32_t code_point)
  {
    return std::ranges::to<R> (std::views::single (code_point));
  }

template <std::ranges::input_range R>
requires std::same_as<char16_t, std::ranges::range_value_t<R>>
  constexpr std::size_t
  utf16::front_mblen (R &&seq)
  {
    if (std::ranges::empty (seq))
      return 0;

    if (auto const first_unit = *std::ranges::cbegin (seq);
        is_high_surrogate (first_unit))
      {
        if (std::ranges::size (seq) == 1)
          return 0;

        char32_t const code_point = surrogate_pair_to_code_point (
          first_unit,
          *std::ranges::next (std::ranges::cbegin (seq)) );

        return is_non_bmp_code_point (code_point) ? 2 : 0;
      }
    else
      return is_bmp_code_point (first_unit) ? 1 : 0;
  }

template <std::ranges::input_range R>
requires std::same_as<char16_t, std::ranges::range_value_t<R>>
  constexpr char32_t
  utf16::to_code_point (R &&seq)
  {
    if (auto const first_unit = *std::ranges::cbegin (seq);
        is_high_surrogate (first_unit))
      return surrogate_pair_to_code_point (first_unit,
                                           *std::ranges::next (std::ranges::cbegin (seq)));
    else
      return static_cast<char32_t> (first_unit);
  }

template <std::ranges::range R>
requires std::same_as<char16_t, std::ranges::range_value_t<R>>
  constexpr R
  utf16::code_point_to (char32_t const code_point)
  {
    auto const arr = code_point_to_surrogate_pair (code_point);
    return std::ranges::to<R> (arr | std::views::take (is_bmp_code_point (code_point) ? 1 : 2));
  }

constexpr char32_t
utf16::surrogate_pair_to_code_point (char16_t const high,
                                     char16_t const low) noexcept
{
  return (static_cast<char32_t> (high - high_surrogate_range.start) << 10)
         | static_cast<char32_t> (low - low_surrogate_range.start)
         + 0x10000U;
}

constexpr std::array<char16_t, 2>
utf16::code_point_to_surrogate_pair (char32_t code_point)
{
  code_point -= 0x10000;
  return std::to_array ({ static_cast<char16_t> ((code_point >> 10) + high_surrogate_range.start),
                          static_cast<char16_t> ((code_point & 0x3FF) + low_surrogate_range.start) });
}

constexpr bool
utf16::is_high_surrogate (char16_t const code_unit) noexcept
{
  return high_surrogate_range.start <= code_unit && code_unit < high_surrogate_range.end;
}

constexpr bool
utf16::is_low_surrogate (char16_t const code_unit) noexcept
{
  return low_surrogate_range.start <= code_unit && code_unit < low_surrogate_range.end;
}

constexpr bool
utf16::is_bmp_code_point (char32_t const code_point) noexcept
{
  std::size_t low = 0;
  std::size_t high = bmp_assigned_ranges.size () - 1;
  while (low <= high)
    if (std::size_t const mid = low + (high - low) / 2; code_point < bmp_assigned_ranges[mid].start)
      {
        if (mid == 0)
          return false;
        high = mid - 1;
      }
    else if (bmp_assigned_ranges[mid].end <= code_point)
      low = mid + 1;
    else
      return true;

  return false;
}

constexpr bool
utf16::is_non_bmp_code_point (char32_t const code_point) noexcept
{
  std::size_t low = 0;
  std::size_t high = non_bmp_assigned_ranges.size () - 1;
  while (low <= high)
    if (std::size_t const mid = low + (high - low) / 2; code_point < non_bmp_assigned_ranges[mid].start)
      {
        if (mid == 0)
          return false;
        high = mid - 1;
      }
    else if (non_bmp_assigned_ranges[mid].end <= code_point)
      low = mid + 1;
    else
      return true;

  return false;
}

template <std::ranges::input_range R>
requires std::same_as<char8_t, std::ranges::range_value_t<R>>
  constexpr std::size_t
  utf8::front_mblen (R &&seq)
  {
    if (std::ranges::empty (seq))
      return 0;

    auto const trivial_mblen = trivial_mblen_from_unit (*std::ranges::cbegin (seq));
    if (0 == trivial_mblen || std::ranges::size (seq) < trivial_mblen)
      return 0;

    char32_t code_point = extract_bits_from_code_unit (*std::ranges::cbegin (seq), trivial_mblen);
    auto current = std::ranges::cbegin (seq);
    for (std::size_t i = 0; i < trivial_mblen - 1; ++i)
      {
        std::ranges::advance (current, 1);
        if (is_continuation_unit (*current))
          code_point = code_point << 6 | extract_bits_from_code_unit (*current, from_continuation_byte);
        else
          return 0;
      }

    auto const assigned_ranges =
      trivial_mblen == 1 ? assigned_ranges_1.data () :
      trivial_mblen == 2 ? assigned_ranges_2.data () :
      trivial_mblen == 3 ? assigned_ranges_3.data () :
      assigned_ranges_4.data ();

    std::size_t low = 0;
    std::size_t high = trivial_mblen == 1 ? assigned_ranges_1.size () :
                       trivial_mblen == 2 ? assigned_ranges_2.size () :
                       trivial_mblen == 3 ? assigned_ranges_3.size () :
                       assigned_ranges_4.size ();
    while (low <= high)
      {
        if (std::size_t const mid = low + (high - low) / 2;
            code_point < assigned_ranges[mid].start)
          {
            if (mid == 0)
              return 0;
            high = mid - 1;
          }
        else if (assigned_ranges[mid].end <= code_point)
          low = mid + 1;
        else
          return trivial_mblen;
      }

    return 0;
  }

template <std::ranges::input_range R>
requires std::same_as<char8_t, std::ranges::range_value_t<R>>
  constexpr char32_t
  utf8::to_code_point (R &&seq)
  {
    auto const trivial_mblen = trivial_mblen_from_unit (*std::ranges::cbegin (seq));

    char32_t code_point = extract_bits_from_code_unit (*std::ranges::cbegin (seq), trivial_mblen);

    auto current = std::ranges::cbegin (seq);
    for (std::size_t i = 0; i < trivial_mblen - 1; ++i)
      {
        std::ranges::advance (current, 1);
        code_point <<= 6;
        code_point |= extract_bits_from_code_unit (*current, from_continuation_byte);
      }

    return code_point;
  }

template <std::ranges::range R>
requires std::same_as<char8_t, std::ranges::range_value_t<R>>
  constexpr R
  utf8::code_point_to (char32_t const code_point)
  {
    // Mr. Gemini wrote this branchless logic. I believe in them. They're much
    // smarter than me, you know.

    std::size_t const len = 1 + (code_point > 0x7FU) + (code_point > 0x7FFU) + (code_point > 0xFFFFU);

    std::array<char8_t, 4> result_bytes {};

    result_bytes[0] = static_cast<char8_t>(
      ((len == 1) * (code_point & 0x7FU))
      | ((len == 2) * (0xC0U | ((code_point >> 6) & 0x1FU)))
      | ((len == 3) * (0xE0U | ((code_point >> 12) & 0x0FU)))
      | ((len == 4) * (0xF0U | ((code_point >> 18) & 0x07U))) );

    result_bytes[1] = static_cast<char8_t>(
      (len >= 2)
      * (0x80U
         | (((len == 2) * (code_point & 0x3FU))
            | ((len == 3) * ((code_point >> 6) & 0x3FU))
            | ((len == 4) * ((code_point >> 12) & 0x3FU)) )));

    result_bytes[2] = static_cast<char8_t>(
      (len >= 3)
      * (0x80U
         | (((len == 3) * (code_point & 0x3FU))
            | ((len == 4) * ((code_point >> 6) & 0x3FU)) )));

    result_bytes[3] = static_cast<char8_t>(
      (len == 4) * (0x80U | (code_point & 0x3FU)) );

    return std::ranges::to<R> (result_bytes | std::views::take (len));
  }

constexpr std::size_t
utf8::trivial_mblen_from_unit (char8_t const unit) noexcept
{
  if (unit <= 0x7F)
    return 1;
  if (0xC2 <= unit && unit <= 0xDF)
    return 2;
  if (0xE0 <= unit && unit <= 0xEF)
    return 3;
  if (0xF0 <= unit && unit <= 0xF4)
    return 4;
  return 0;
}

constexpr char32_t
utf8::extract_bits_from_code_unit (char8_t const code_unit,
                                   std::size_t const trivial_mblen) noexcept
{
  switch (trivial_mblen)
    {
    case from_continuation_byte:
      return code_unit & 0x3F;
    case 1:
      return code_unit & 0x7F;
    case 2:
      return code_unit & 0x1F;
    case 3:
      return code_unit & 0x0F;
    case 4:
      return code_unit & 0x07;
    default:
      std::unreachable ();
    }
}

constexpr bool
utf8::is_continuation_unit (char8_t const code_unit) noexcept
{
  return (code_unit & 0xC0) == 0x80;
}

} // namespace char_db