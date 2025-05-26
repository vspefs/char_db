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

#include "std_expo.hh"

namespace char_db::std_expo {

// class non_propagating_cache

template <typename T>
requires std::is_object_v<T>
  constexpr
  non_propagating_cache<T>::non_propagating_cache (non_propagating_cache const &) noexcept
  {
  }

template <typename T>
requires std::is_object_v<T>
  constexpr
  non_propagating_cache<T>::non_propagating_cache (non_propagating_cache &&other) noexcept
  {
    other.reset ();
  }

template <typename T>
requires std::is_object_v<T>
  constexpr non_propagating_cache<T> &
  non_propagating_cache<T>::operator= (non_propagating_cache const &other) noexcept
  {
    if (std::addressof (other) != this)
      std::optional<T>::reset ();
    return *this;
  }

template <typename T>
requires std::is_object_v<T>
  constexpr non_propagating_cache<T> &
  non_propagating_cache<T>::operator= (non_propagating_cache &&other) noexcept
  {
    std::optional<T>::reset ();
    other.reset ();
    return *this;
  }

template <typename T>
requires std::is_object_v<T>
  template <typename I>
    constexpr T &
    non_propagating_cache<T>::emplace_deref (I const& i)
    {
      std::optional<T>::emplace (*i);
      return **this;
    }

// class movable_box

template <typename T>
requires std::move_constructible<T> && std::is_object_v<T>
  constexpr
  movable_box<T>::movable_box () noexcept (std::is_nothrow_default_constructible_v<T>)
                                 requires std::default_initializable<T>
  : movable_box (std::in_place)
  {
  }

template <typename T>
requires std::move_constructible<T> && std::is_object_v<T>
  constexpr movable_box<T> &
  movable_box<T>::operator= (movable_box const &other) noexcept (std::is_nothrow_copy_constructible_v<T>)
                                                       requires std::copy_constructible<T>
  {
    if (this != std::addressof (other))
      {
        if (other)
          std::optional<T>::emplace (*other);
        else
          std::optional<T>::reset ();
      }
    return *this;
  }

template <typename T>
requires std::move_constructible<T> && std::is_object_v<T>
  constexpr movable_box<T> &
  movable_box<T>::operator= (movable_box<T> &&other) noexcept (std::is_nothrow_move_constructible_v<T>)
  {
    if (this != std::addressof (other))
      {
        if (other)
          std::optional<T>::emplace (std::move (*other));
        else
          std::optional<T>::reset ();
      }
    return *this;
  }

} // namespace char_db::std_expo