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

#include <ranges>
#include <iterator>
#include <algorithm>
#include <concepts>
#include <type_traits>
#include "database.hh"
#include "std_expo.hh"

namespace char_db {

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  class decoding_view : public std::ranges::view_interface<decoding_view<Db, V>>
  {
  public:
    class iterator
    {
    public:
      using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
      using difference_type = std::ranges::range_difference_t<V>;
      using iterator_concept = std::conditional_t<std::ranges::bidirectional_range<V>,
                                                  std::bidirectional_iterator_tag,
                                                  std::forward_iterator_tag>;
      friend class decoding_view;
    public:
      iterator () = default;

      constexpr value_type operator* () const;
      constexpr iterator &operator++ ();
      constexpr iterator operator++ (int);
      constexpr iterator &operator-- () requires std::ranges::bidirectional_range<V>;
      constexpr iterator operator-- (int) requires std::ranges::bidirectional_range<V>;

      friend constexpr bool operator== (iterator const &x, iterator const &y)
      {
        return x.current_ == y.next_;
      }
      friend constexpr bool operator== (iterator const &x, std::default_sentinel_t)
      {
        return x.current_ == x.next_;
      }
    private:
      constexpr iterator (decoding_view &, std::ranges::iterator_t<V>, std::ranges::iterator_t<V>);

      decoding_view *parent_;
      std::ranges::iterator_t<V> current_;
      std::ranges::iterator_t<V> next_;
    };
  public:
    decoding_view () requires std::default_initializable<V> = default;
    constexpr explicit decoding_view (V);

    constexpr V base () const & requires std::copy_constructible<V>;
    constexpr V base () &&;
    constexpr iterator begin ();
    constexpr auto end ();
  private:
    constexpr std::ranges::iterator_t<V> find_next (std::ranges::iterator_t<V>);
    constexpr std::ranges::iterator_t<V> find_prev (std::ranges::iterator_t<V>) requires std::ranges::bidirectional_range<V>;
    V base_;
    std_expo::non_propagating_cache<std::ranges::iterator_t<V>> begin_;
  };

} // namespace char_db

namespace char_db::views {

namespace _detail {

template<typename Db>
struct _decoding_adaptor : public std::ranges::range_adaptor_closure<_decoding_adaptor<Db>>
  {
    template <std::ranges::viewable_range R>
      constexpr auto operator() (R &&r) const;
  };

}

template<typename Db>
  inline constexpr _detail::_decoding_adaptor<Db> decoding {};

} // namespace char_db::views

#include "views.tcc"
