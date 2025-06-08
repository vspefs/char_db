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

#include "views.hh"

namespace char_db {

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator::iterator (decoding_view &parent,
                                                      std::ranges::iterator_t<V> current,
                                                      std::ranges::iterator_t<V> next)
  : parent_ (std::addressof (parent)),
    current_ (std::move (current)),
    next_ (std::move (next))
  {
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator::value_type
  decoding_view<Db, V>::iterator::operator* () const
  {
    return std::ranges::subrange (current_, next_);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator &
  decoding_view<Db, V>::iterator::operator++ ()
  {
    current_ = next_;
    next_ = parent_->find_next (current_);
    return *this;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator
  decoding_view<Db, V>::iterator::operator++ (int)
  {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator &
  decoding_view<Db, V>::iterator::operator-- () requires std::ranges::bidirectional_range<V>
  {
    next_ = current_;
    current_ = parent_->find_prev (current_);
    return *this;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator
  decoding_view<Db, V>::iterator::operator-- (int) requires std::ranges::bidirectional_range<V>
  {
    auto tmp = *this;
    --*this;
    return tmp;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::decoding_view (V base)
  : base_ (std::move (base))
  {
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr V
  decoding_view<Db, V>::base () const & requires std::copy_constructible<V>
  {
    return base_;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr V
  decoding_view<Db, V>::base () &&
  {
    return std::move (base_);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoding_view<Db, V>::iterator
  decoding_view<Db, V>::begin ()
  {
    std::ranges::iterator_t<V> iter;

    if (begin_.has_value ())
      iter = begin_.value ();
    else
      {
        iter = find_next (std::ranges::begin (base ()));
        begin_.emplace (iter);
      }

    return iterator (*this, std::ranges::begin (base ()), iter);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr auto
  decoding_view<Db, V>::end ()
  {
    if constexpr (std::ranges::common_range<V>)
      return iterator (*this, std::ranges::end (base_), std::ranges::end (base_));
    else
      return std::default_sentinel;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr std::ranges::iterator_t<V>
  decoding_view<Db, V>::find_next (std::ranges::iterator_t<V> current)
  {
    auto const subseq = std::ranges::subrange (current, std::ranges::end (base_));
    if (auto const mblen = Db::front_mblen (subseq);
        mblen > 0)
      {
        auto const next = std::ranges::next (current, mblen, std::ranges::end (base_));
        if (std::ranges::end (base_) != next)
          return next;
      }
    return std::ranges::end (base_);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr std::ranges::iterator_t<V>
  decoding_view<Db, V>::find_prev (std::ranges::iterator_t<V> current) requires std::ranges::bidirectional_range<V>
  {
    for (auto const probe : std::ranges::subrange (std::make_reverse_iterator (current), std::ranges::rend (base_)))
      {
        auto const iter = probe.base ();
        if (auto const subseq = std::ranges::subrange (iter, current);
            Db::is_valid_char (subseq))
          return iter;
      }
    return std::ranges::begin (base_);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator::iterator (decoded_view &parent,
                                                     std::size_t const rank)
  : parent_ (std::addressof (parent)), rank_ (rank)
  {
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator::value_type
  decoded_view<Db, V>::iterator::operator* () const
  {
    auto const this_iter = std::ranges::next (parent_->base ().begin (),
                                              parent_->book_.select (rank_));
    auto const next_iter = std::ranges::next (parent_->base ().begin (),
                                              parent_->book_.select (rank_ + 1));
    return std::ranges::subrange (this_iter, next_iter);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator &
  decoded_view<Db, V>::iterator::operator++ ()
  {
    if (parent_->end () != *this)
      ++rank_;
    return *this;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator
  decoded_view<Db, V>::iterator::operator++ (int)
  {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator &
  decoded_view<Db, V>::iterator::operator-- ()
  {
    if (std::ranges::begin (parent_->base ()) != *this)
      --rank_;
    return *this;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator
  decoded_view<Db, V>::iterator::operator-- (int)
  {
    auto tmp = *this;
    --(*this);
    return tmp;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::decoded_view (V base)
  : base_ (std::move (base)), book_ ()
  {
    using dynamic_bitset = std::vector<bool>;
    dynamic_bitset book (std::ranges::size (base_), false);

    auto const begin = std::ranges::cbegin (base_);
    auto const end = std::ranges::cend (base_);
    std::size_t index = 0;
    for (auto iter = begin; iter != end; ++iter, ++index)
      if (Db::starts_with_valid_char (std::ranges::subrange (iter, end)))
        book[index] = true;

    book_ = _container::succinct_bitset<std::dynamic_extent> (std::from_range, std::move (book));
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr V
  decoded_view<Db, V>::base () const & requires std::copy_constructible<V>
  {
    return base_;
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr V
  decoded_view<Db, V>::base () &&
  {
    return std::move (base_);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr decoded_view<Db, V>::iterator
  decoded_view<Db, V>::begin ()
  {
    return iterator (*this, 0);
  }

template <typename Db, std::ranges::forward_range V>
requires std::ranges::view<V> && database_of<Db, std::ranges::range_value_t<V>>
  constexpr auto
  decoded_view<Db, V>::end ()
  {
    if constexpr (std::ranges::common_range<V>)
      return iterator (*this, book_.count ());
    else
      return std::default_sentinel;
  }

} // namespace char_db



namespace char_db::views::_detail {

template <typename Db>
  template <std::ranges::viewable_range R>
    constexpr auto
    _decoding_adaptor<Db>::operator() (R &&r) const
    {
      return decoding_view<Db, std::ranges::views::all_t<R>> (std::views::all (std::forward<R> (r)));
    }

template <typename Db>
  template <std::ranges::viewable_range R>
    constexpr auto
    _decoded_adaptor<Db>::operator() (R &&r) const
    {
      return decoded_view<Db, std::ranges::views::all_t<R>> (std::views::all (std::forward<R> (r)));
    }

} // namespace char_db::views::_detail

