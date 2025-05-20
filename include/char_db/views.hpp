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

#include "database.hpp"
#include "std_expo.hpp"

#include <ranges>
#include <optional>

namespace char_db {

template <typename Db, std::ranges::forward_range V>
requires database_of<Db, std::ranges::range_value_t<V>>
  class decoding_view : public std::ranges::view_interface<decoding_view<Db, V>>
  {
  public:
    class iterator
    {
    public:
      using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
      using difference_type = std::ranges::range_difference_t<V>;
      using iterator_category = std::input_iterator_tag;
      using iterator_concept = std::conditional_t<std::ranges::bidirectional_range<V>,
                                                  std::bidirectional_iterator_tag,
                                                  std::forward_iterator_tag>;
      friend class decoding_view;

    public:
      iterator () = default;

      constexpr value_type
      operator* () const
      {
        return value_type (current, next);
      }
      constexpr iterator
      &operator++ ()
      {
        current = next;
        next = parent->find_next (current);
        return *this;
      }
      constexpr iterator
      operator++ (int)
      {
        auto tmp = *this;
        ++*this;
        return tmp;
      }
      constexpr iterator
      &operator-- () requires std::ranges::bidirectional_range<V>
      {
        next = current;
        current = parent->find_prev (next);
        return *this;
      }
      constexpr iterator
      operator-- (int) requires std::ranges::bidirectional_range<V>
      {
        auto tmp = *this;
        --*this;
        return tmp;
      }

      friend constexpr bool
      operator== (iterator const &x, iterator const &y)
      {
        return x.current == y.current;
      }
      friend constexpr bool
      operator== (iterator const &x, std::default_sentinel_t)
      {
        return x.current == x.next;
      }
    private:
      constexpr
      iterator (decoding_view &parent, std::ranges::iterator_t<V> current,
                std::ranges::iterator_t<V> next)
      : parent (std::addressof (parent)), current (current), next (next)
      {
      }

    private:
      decoding_view *parent;
      std::ranges::iterator_t<V> current;
      std::ranges::iterator_t<V> next;
    };

  public:
    decoding_view () requires std::default_initializable<V> = default;

    constexpr explicit
    decoding_view (V base_view)
    : base_view (std::move (base_view))
    {
    }

    constexpr V
    base () const & requires std::copy_constructible<V>
    {
      return base_view;
    }

    constexpr V
    base () &&
    {
      return std::move (base_view);
    }

    constexpr iterator
    begin ()
    {
      std::ranges::iterator_t<V> iter;

      if (cached_begin.has_value ())
        iter = cached_begin.value ();
      else
        {
          iter = find_next (std::ranges::begin (base ()));
          cached_begin.emplace (iter);
        }

      return iterator (*this, std::ranges::begin (base ()), iter);
    }

    constexpr iterator
    end ()
    {
      if constexpr (std::ranges::common_range<V>)
        return iterator (*this, std::ranges::end (base_view), std::ranges::end (base_view));
      else
        return std::default_sentinel;
    }

  private:
    constexpr std::ranges::iterator_t<V>
    find_next (std::ranges::iterator_t<V> current)
    // pre (Db::starts_with_valid_char (std::ranges::subrange (current,
    //                                                         std::ranges::end (base_view) )))
    {
      return std::ranges::next (
        current,
        Db::front_mblen (std::ranges::subrange (current, std::ranges::end (base_view))),
        std::ranges::end (base_view) );
    }

    constexpr std::ranges::iterator_t<V>
    find_prev (std::ranges::iterator_t<V> current) requires std::ranges::bidirectional_range<V>
    {
      for (auto const rev_subrange = std::ranges::subrange (
             std::make_reverse_iterator (current),
             std::ranges::rend (base_view) );
	   auto rev_iter : rev_subrange)
        {
          auto const iter = std::make_reverse_iterator (rev_iter);
	  if (auto const subrange = std::ranges::subrange (
	        iter,
	        std::ranges::end (base_view) );
	      current == std::ranges::next (iter, Db::front_mblen (subrange)))
	    return iter;
        }

      return std::ranges::end (base_view);
    }

    V base_view;
    std_expo::non_propagating_cache<std::ranges::iterator_t<V>> cached_begin;
  };

} // namespace char_db


namespace char_db::views {

template <typename Db>
  struct decoding_adaptor
  {
    template <std::ranges::viewable_range R>
      static constexpr auto operator() (R &&r)
    {
      return decoding_view<Db, std::views::all_t<R>> (std::forward<R> (r));
    };
  };

template <typename Db>
  constexpr auto
  decoding ()
  {
    return decoding_adaptor<Db> ();
  }

template <typename Db, std::ranges::viewable_range R>
  constexpr auto
  decoding (R &&r)
  {
    return decoding_adaptor<Db> () (r);
  }

template <std::ranges::viewable_range R, typename Db>
  constexpr auto
  operator| (R &&r, decoding_adaptor<Db> const &adaptor)
  {
    return adaptor (std::forward<R> (r));
  }

} // namespace char_db::views