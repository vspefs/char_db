#pragma once

#include <concepts>
#include <ranges>
#include "../std_expo.hh"

// Yet another fixed string implementation.

// Let's hope P3094 make its way to the standard soon.

namespace char_db::_container {

template <typename CharT, std::size_t N>
  class basic_fixed_string
  {
  public:
    using value_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using pointer = value_type *;
    using const_pointer = value_type const *;
    using reference = value_type &;
    using const_reference = value_type const &;

    class iterator
    {
    public:
      using value_type = CharT;
      using difference_type = std::ptrdiff_t;
      using iterator_concept = std::contiguous_iterator_tag;
      friend class basic_fixed_string;
    public:
      constexpr iterator () noexcept = default;

      constexpr value_type operator* () const noexcept;
      constexpr iterator &operator++ () noexcept;
      constexpr iterator operator++ (int) noexcept;
      constexpr iterator &operator-- () noexcept;
      constexpr iterator operator-- (int) noexcept;
      constexpr bool operator== (iterator const &) const noexcept;
    private:
      constexpr explicit iterator (value_type const *ptr) noexcept;
      value_type const *ptr_ = nullptr;
    };
    using const_iterator = iterator;

    static constexpr std::integral_constant<size_type, N> size = {};
    static constexpr std::integral_constant<size_type, N> max_size = {};
    static constexpr std::bool_constant<0 == N> empty = {};
  public:
    consteval explicit(false) basic_fixed_string (CharT const (&)[N + 1]) noexcept;

    constexpr basic_fixed_string (std::from_range_t, std_expo::container_compatible_range<value_type> auto &&);

    constexpr basic_fixed_string (basic_fixed_string const &) noexcept = default;
    constexpr basic_fixed_string &operator= (basic_fixed_string const &) noexcept = default;

    constexpr const_iterator begin () const noexcept;
    constexpr const_iterator end () const noexcept;

    constexpr const_pointer c_str () const noexcept;
    constexpr const_pointer data () const noexcept;
  public:
    CharT data_[N + 1] = {};
  };

template<typename CharT, size_t N>
  basic_fixed_string(CharT const (&str)[N]) -> basic_fixed_string<CharT, N - 1>;

template<typename CharT, size_t N>
  basic_fixed_string(std::from_range_t, std::array<CharT, N>) -> basic_fixed_string<CharT, N>;

template <std::size_t N> using fixed_string = basic_fixed_string<char, N>;
template <std::size_t N> using fixed_wstring = basic_fixed_string<wchar_t, N>;
template <std::size_t N> using fixed_u8string = basic_fixed_string<char8_t, N>;
template <std::size_t N> using fixed_u16string = basic_fixed_string<char16_t, N>;
template <std::size_t N> using fixed_u32string = basic_fixed_string<char32_t, N>;

} // namespace char_db::_container

#include "fixed_string.tcc"