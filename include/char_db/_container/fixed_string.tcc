#pragma once

#include "fixed_string.hh"

namespace char_db::_container {

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::iterator::iterator (value_type const *ptr) noexcept
  : ptr_ (ptr)
  {
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::iterator::value_type
  basic_fixed_string<CharT, N>::iterator::operator* () const noexcept
  {
    return *ptr_;
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::iterator &
  basic_fixed_string<CharT, N>::iterator::operator++ () noexcept
  {
    ++ptr_;
    return *this;
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::iterator
  basic_fixed_string<CharT, N>::iterator::operator++ (int) noexcept
  {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::iterator &
  basic_fixed_string<CharT, N>::iterator::operator-- () noexcept
  {
    --ptr_;
    return *this;
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::iterator
  basic_fixed_string<CharT, N>::iterator::operator-- (int) noexcept
  {
    auto tmp = *this;
    --(*this);
    return tmp;
  }

template <typename CharT, std::size_t N>
  constexpr bool
  basic_fixed_string<CharT, N>::iterator::operator== (iterator const &other) const noexcept
  {
    return ptr_ == other.ptr_;
  }


template <typename CharT, std::size_t N>
  consteval basic_fixed_string<CharT, N>::basic_fixed_string (CharT const (&txt)[N + 1]) noexcept
  {
    std::ranges::copy_n (txt, N, data_);
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::basic_fixed_string (
      std::from_range_t,std_expo::container_compatible_range<value_type> auto &&r)
  {
    std::ranges::copy_n (std::ranges::cbegin (r), N, data_);
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::const_iterator
  basic_fixed_string<CharT, N>::begin () const noexcept
  {
    return const_iterator (static_cast<value_type const *> (data_));
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::const_iterator
  basic_fixed_string<CharT, N>::end () const noexcept
  {
    return const_iterator (static_cast<value_type const *> (data_) + N);
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::const_pointer
  basic_fixed_string<CharT, N>::c_str () const noexcept
  {
    return data_;
  }

template <typename CharT, std::size_t N>
  constexpr basic_fixed_string<CharT, N>::const_pointer
  basic_fixed_string<CharT, N>::data () const noexcept
  {
    return data_;
  }

} // namespace char_db::_container