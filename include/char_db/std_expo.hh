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

#include <type_traits>
#include <optional>
#include <utility>

namespace char_db::std_expo {

template <typename T>
requires std::is_object_v<T>
  class non_propagating_cache : public std::optional<T>
  {
  public:
    constexpr non_propagating_cache () noexcept = default;
    constexpr non_propagating_cache (non_propagating_cache const &) noexcept;
    constexpr non_propagating_cache (non_propagating_cache &&other) noexcept;

    constexpr non_propagating_cache &operator= (non_propagating_cache const &other) noexcept;
    constexpr non_propagating_cache &operator= (non_propagating_cache &&other) noexcept;

    template <typename I>
      constexpr T &emplace_deref (I const& i);
  };

template <typename T>
requires std::move_constructible<T> && std::is_object_v<T>
  class movable_box : public std::optional<T>
  {
  public:
    constexpr movable_box ()
      noexcept (std::is_nothrow_default_constructible_v<T>)
      requires std::default_initializable<T>;

    constexpr movable_box &operator= (movable_box const &other)
      noexcept (std::is_nothrow_copy_constructible_v<T>)
      requires std::copy_constructible<T>;

    constexpr movable_box &operator= (movable_box &&other)
      noexcept (std::is_nothrow_move_constructible_v<T>);
  };

template <typename R, typename T>
  concept container_compatible_range =
    std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;

} // namespace char_db::std_expo

#include "std_expo.tcc"