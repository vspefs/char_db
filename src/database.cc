export module vspefs.char_db : database;

import std;


// concepts & interfaces
namespace char_db {

template <typename T>
  concept minimal_database_interface = requires (
      std::vector<typename T::char_type> seq,
      char32_t code_point)
  {
    // For all APIs specified in this concept, their precondition 
    // is that the input sequences must be non-empty.

    typename T::char_type;

    // front_mblen () returns 0 when seq does not begin with a valid character
    { T::front_mblen (seq) } -> std::same_as<std::size_t>;
    { T::to_code_point (seq) } -> std::same_as<char32_t>;
    // code_unit_size () returns 0 when code_point is not valid
    { T::code_unit_size (code_point) } -> std::same_as<std::size_t>;
    { T::code_point_on (code_point, std::span (seq)) } -> std::same_as<void>;
  };

export template <typename T, typename CharT> concept database_of =
    minimal_database_interface<T> && requires (std::vector<CharT> seq, char32_t code_point)
    {
      // For all APIs specified in this concept, their precondition 
      // is that the input sequences must be non-empty.

      requires std::same_as<typename T::char_type, CharT>;

      // char_size () counts until invalid character or end of sequence
      { T::char_size (seq) } -> std::same_as<std::size_t>;
      { T::is_valid_char (seq) } -> std::same_as<bool>;
      { T::starts_with_valid_char (seq) } -> std::same_as<bool>;
      { T::validate_char_sequence (seq) } -> std::same_as<bool>;

      // TODO: make it allocator-aware; write a documentation on its specific behavior
      { T::template code_point_to<std::vector<typename T::char_type>> (code_point) }
        -> std::same_as<std::vector<typename T::char_type>>;
    };

export template <typename D, typename CharT>
  class database_interface
  {
  public:
    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    static constexpr bool is_valid_char (R &&);

    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    static constexpr std::size_t char_size (R &&);

    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    static constexpr bool starts_with_valid_char (R &&);

    template <std::ranges::input_range R>
    requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    static constexpr bool validate_char_sequence (R &&);

    template <std::ranges::range R>
    requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    static constexpr R code_point_to (char32_t);
  };

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    constexpr bool
    database_interface<D, CharT>::is_valid_char (R &&seq)
    {
      if (auto const mblen = D::front_mblen (seq);
          0 != mblen && std::ranges::size (seq) == mblen)
        return true;
      else
        return false;
    }

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    constexpr std::size_t
    database_interface<D, CharT>::char_size (R &&seq)
    {
      auto const sentinel = std::ranges::cend (seq);
      auto cursor = std::ranges::cbegin (seq);
      std::size_t size = 0, mblen = 0;

      while (sentinel != cursor)
        {
          mblen = D::front_mblen (std::ranges::subrange (cursor, sentinel));
          if (0 == mblen)
            break;

          std::ranges::advance (cursor, mblen);
          size++;
        }

      return size;
    }

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    constexpr bool
    database_interface<D, CharT>::starts_with_valid_char (R &&seq)
    {
      return 0 != D::front_mblen (seq);
    }

template <typename D, typename CharT>
  template <std::ranges::input_range R>
  requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    constexpr bool
    database_interface<D, CharT>::validate_char_sequence (R &&seq)
    {
      auto const sentinel = std::ranges::cend (seq);
      auto cursor = std::ranges::cbegin (seq);
      std::size_t mblen = 0;

      while (sentinel != cursor)
        {
          mblen = D::front_mblen (std::ranges::subrange (cursor, sentinel));
          if (0 == mblen)
            return false;

          std::ranges::advance (cursor, mblen);
        }

      return true;
    }

template <typename D, typename CharT>
  template <std::ranges::range R>
  requires std::same_as<CharT, std::remove_cvref_t<std::ranges::range_value_t<R>>>
    constexpr R
    database_interface<D, CharT>::code_point_to (char32_t const code_point)
    {
      auto const len = D::code_unit_size (code_point);
      auto tmp = std::vector<CharT> (len);
      D::code_point_on (code_point, tmp);
      return std::move (tmp) | std::ranges::to<R>;
    }

} // namespace char_db


// databases
namespace char_db {

export class utf32 : public database_interface<utf32, char32_t>
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
  requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  static constexpr std::size_t front_mblen (R &&seq);

  template <std::ranges::input_range R>
  requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  static constexpr char32_t to_code_point (R &&seq);

  static constexpr std::size_t code_unit_size (char32_t);

  template <std::size_t Extent = std::dynamic_extent>
  static constexpr void code_point_on (char32_t, std::span<char_type, Extent>);

  static constexpr bool is_valid_code_point (char32_t code_point);

private:
  static constexpr auto assigned_ranges = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/just_ranges.inc>
#pragma clang diagnostic pop
      });
};

export class utf16 : public database_interface<utf16, char16_t>
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

  struct surrogate_pair_t
  {
    char16_t const high;
    char16_t const low;
  };

public:
  template <std::ranges::input_range R>
  requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  static constexpr std::size_t front_mblen (R &&seq);

  template <std::ranges::input_range R>
  requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr char32_t to_code_point (R &&);

  static constexpr std::size_t code_unit_size (char32_t);

  template <std::size_t Extent = std::dynamic_extent>
  static constexpr void code_point_on (char32_t, std::span<char_type, Extent>);

  static constexpr char32_t surrogate_pair_to_code_point (surrogate_pair_t) noexcept;

  static constexpr surrogate_pair_t code_point_to_surrogate_pair (char32_t code_point);

  static constexpr bool is_high_surrogate (char_type code_unit) noexcept;

  static constexpr bool is_low_surrogate (char_type code_unit) noexcept;

  static constexpr bool is_bmp_code_point (char32_t code_point) noexcept;

  static constexpr bool is_non_bmp_code_point (char32_t  code_point) noexcept;

private:
  static constexpr auto bmp_assigned_ranges = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/bmp_ranges.inc>
#pragma clang diagnostic pop
      });
  static constexpr auto non_bmp_assigned_ranges = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/non_bmp_ranges.inc>
#pragma clang diagnostic pop
      });
  static constexpr auto high_surrogate_range = surrogate_range_t { 0xD800u, 0xDC00u };
  static constexpr auto low_surrogate_range = surrogate_range_t { 0xDC00u, 0xE000u };
};

export class utf8 : public database_interface<utf8, char8_t>
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
  requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  static constexpr std::size_t front_mblen (R &&seq);

  template <std::ranges::input_range R>
  requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  static constexpr char32_t to_code_point (R &&seq);

  static constexpr std::size_t code_unit_size (char32_t);

  template <std::size_t Extent = std::dynamic_extent>
  static constexpr void code_point_on (char32_t, std::span<char_type, Extent>);

  static constexpr std::size_t trivial_mblen_from_unit (char_type unit) noexcept;
  static constexpr char32_t extract_bits_from_code_unit (char_type code_unit, std::size_t trivial_mblen) noexcept;
  static constexpr bool is_continuation_unit (char_type code_unit) noexcept;

private:
  static constexpr auto assigned_ranges_1 = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/utf8_ranges_1.inc>
#pragma clang diagnostic pop
      });
  static constexpr auto assigned_ranges_2 = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/utf8_ranges_2.inc>
#pragma clang diagnostic pop
      });
  static constexpr auto assigned_ranges_3 = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/utf8_ranges_3.inc>
#pragma clang diagnostic pop
      });
  static constexpr auto assigned_ranges_4 = std::to_array<assigned_range_t> ({
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#include <char_db/generated/utf8_ranges_4.inc>
#pragma clang diagnostic pop
      });
};

template <std::ranges::input_range R>
requires std::same_as<char32_t, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr std::size_t
  utf32::front_mblen (R &&seq)
  {
    return utf32::code_unit_size (*std::ranges::cbegin (seq));
  }

template <std::ranges::input_range R>
requires std::same_as<char32_t, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr char32_t
  utf32::to_code_point (R &&seq)
  {
    return *std::ranges::cbegin (seq);
  }

constexpr std::size_t
utf32::code_unit_size (char32_t const code_point)
{
  return is_valid_code_point (code_point) ? 1 : 0;
}

template <std::size_t Extent = std::dynamic_extent>
  constexpr void
  utf32::code_point_on (char32_t const code_point, std::span<char32_t, Extent> const dest)
  {
    dest[0] = code_point;
  }

constexpr bool
utf32::is_valid_code_point (char32_t const code_point)
{
  std::size_t low = 0;
  std::size_t high = assigned_ranges.size () - 1;
  while (low <= high)
    if (std::size_t const mid = low + (high - low) / 2;
        code_point < assigned_ranges[mid].start)
      {
        if (0 == mid)
          return 0;
        high = mid - 1;
      }
    else if (assigned_ranges[mid].end <= code_point)
      low = mid + 1;
    else
      return true;

  return false;
}

template <std::ranges::input_range R>
requires std::same_as<char16_t, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr std::size_t
  utf16::front_mblen (R &&seq)
  {
    if (auto const first_iter = std::ranges::cbegin (seq);
        is_high_surrogate (*first_iter))
      {
        if (std::ranges::size (seq) < 2)
          return 0;

        char32_t const code_point = surrogate_pair_to_code_point ({
            *first_iter,
            *std::ranges::next (first_iter) });

        return is_non_bmp_code_point (code_point) ? 2 : 0;
      }
    else
      return is_bmp_code_point (static_cast<char32_t> (*first_iter)) ? 1 : 0;
  }

template <std::ranges::input_range R>
requires std::same_as<char16_t, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr char32_t
  utf16::to_code_point (R &&seq)
  {
    if (auto const first_iter = std::ranges::cbegin (seq);
        is_high_surrogate (*first_iter))
      return surrogate_pair_to_code_point ({ *first_iter,
                                             *std::ranges::next (first_iter) });
    else
      return static_cast<char32_t> (*first_iter);
  }

constexpr std::size_t
utf16::code_unit_size (char32_t const code_point)
{
  if (is_bmp_code_point (code_point))
    return 1;

  if (is_non_bmp_code_point (code_point))
    return 2;

  return 0;
}

template <std::size_t Extent = std::dynamic_extent>
  constexpr void
  utf16::code_point_on (char32_t const code_point, std::span<char16_t, Extent> const dest)
  {
    switch (code_unit_size (code_point))
      {
      default:
        std::unreachable ();
      case 1:
        dest[0] = static_cast<char16_t> (code_point);
        break;
      case 2:
        auto const surrogate_pair = code_point_to_surrogate_pair (code_point);
        dest[0] = surrogate_pair.high;
        dest[1] = surrogate_pair.low;
        break;
      }
  }

constexpr char32_t
utf16::surrogate_pair_to_code_point (surrogate_pair_t const pair) noexcept
{
  return ((static_cast<char32_t> (pair.high - high_surrogate_range.start) << 10)
          | static_cast<char32_t> (pair.low - low_surrogate_range.start))
         + 0x10000U;
}

constexpr utf16::surrogate_pair_t
utf16::code_point_to_surrogate_pair (char32_t code_point)
{
  code_point -= 0x10000;
  return { static_cast<char16_t> ((code_point >> 10) + high_surrogate_range.start),
           static_cast<char16_t> ((code_point & 0x3FF) + low_surrogate_range.start) };
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
        if (0 == mid)
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
    if (std::size_t const mid = low + (high - low) / 2;
        code_point < non_bmp_assigned_ranges[mid].start)
      {
        if (0 == mid)
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
requires std::same_as<char8_t, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr std::size_t
  utf8::front_mblen (R &&seq)
  {
    using span_type = std::span<assigned_range_t const, std::dynamic_extent>;

    auto const trivial_mblen = trivial_mblen_from_unit (*std::ranges::cbegin (seq));
    if (0 == trivial_mblen || std::ranges::size (seq) < trivial_mblen)
      return 0;

    char32_t code_point = extract_bits_from_code_unit (*std::ranges::cbegin (seq), trivial_mblen);
    auto cursor = std::ranges::cbegin (seq);
    for (std::size_t i = 0; i < trivial_mblen - 1; ++i)
      {
        std::ranges::advance (cursor, 1);
        if (is_continuation_unit (*cursor))
          code_point = code_point << 6 | extract_bits_from_code_unit (*cursor, from_continuation_byte);
        else
          return 0;
      }

    auto const assigned_ranges =
        trivial_mblen == 1 ? span_type (assigned_ranges_1) :
        trivial_mblen == 2 ? span_type (assigned_ranges_2) :
        trivial_mblen == 3 ? span_type (assigned_ranges_3) :
        span_type (assigned_ranges_4);

    std::size_t low = 0;
    std::size_t high = std::ranges::ssize (assigned_ranges) - 1;
    while (low <= high)
      {
        if (std::size_t const mid = low + (high - low) / 2;
            code_point < assigned_ranges[mid].start)
          {
            if (0 == mid)
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
requires std::same_as<char8_t, std::remove_cvref_t<std::ranges::range_value_t<R>>>
  constexpr char32_t
  utf8::to_code_point (R &&seq)
  {
    auto const trivial_mblen = trivial_mblen_from_unit (*std::ranges::cbegin (seq));

    char32_t code_point = extract_bits_from_code_unit (*std::ranges::cbegin (seq), trivial_mblen);

    auto cursor = std::ranges::cbegin (seq);
    for (std::size_t i = 0; i < trivial_mblen - 1; ++i)
      {
        std::ranges::advance (cursor, 1);
        code_point <<= 6;
        code_point |= extract_bits_from_code_unit (*cursor, from_continuation_byte);
      }

    return code_point;
  }

constexpr std::size_t
utf8::code_unit_size (char32_t const code_point)
{
  using span_type = std::span<assigned_range_t const, std::dynamic_extent>;
  
  std::size_t const trivial_size = 0xFFFFU < code_point ? 4 :
                                   0x7FFU < code_point ? 3 :
                                   0x7FU < code_point ? 2 :
                                   1;

  auto const assigned_ranges =
      trivial_size == 1 ? span_type (assigned_ranges_1) :
      trivial_size == 2 ? span_type (assigned_ranges_2) :
      trivial_size == 3 ? span_type (assigned_ranges_3) :
      span_type (assigned_ranges_4);

  std::size_t low = 0;
  std::size_t high = std::ranges::size (assigned_ranges) - 1;
  while (low <= high)
    {
      if (std::size_t const mid = low + (high - low) / 2;
          code_point < assigned_ranges[mid].start)
        {
          if (0 == mid)
            return 0;
          high = mid - 1;
        }
      else if (assigned_ranges[mid].end <= code_point)
        low = mid + 1;
      else
        return trivial_size;
    }

  return 0;
}

template <std::size_t Extent = std::dynamic_extent>
  constexpr void
  utf8::code_point_on (char32_t const code_point, std::span<char8_t, Extent> const dest)
  {
    switch (code_unit_size (code_point))
      {
      default:
        std::unreachable ();
      case 1:
        dest[0] = static_cast<char8_t> (code_point & 0x7FU);
        break;
      case 2:
        dest[0] = static_cast<char8_t> (0xC0U | ((code_point >> 6) & 0x1FU));
        dest[1] = static_cast<char8_t> (0x80U | (code_point & 0x3FU));
        break;
      case 3:
        dest[0] = static_cast<char8_t> (0xE0U | ((code_point >> 12) & 0x0FU));
        dest[1] = static_cast<char8_t> (0x80U | ((code_point >> 6) & 0x3FU));
        dest[2] = static_cast<char8_t> (0x80U | (code_point & 0x3FU));
        break;
      case 4:
        dest[0] = static_cast<char8_t> (0xF0U | ((code_point >> 18) & 0x07U));
        dest[1] = static_cast<char8_t> (0x80U | ((code_point >> 12) & 0x3FU));
        dest[2] = static_cast<char8_t> (0x80U | ((code_point >> 6) & 0x3FU));
        dest[3] = static_cast<char8_t> (0x80U | (code_point & 0x3FU));
        break;
      }
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
    default:
      std::unreachable ();
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
    }
}

constexpr bool
utf8::is_continuation_unit (char8_t const code_unit) noexcept
{
  return 0x80 == (code_unit & 0xC0);
}

} // namespace char_db


// wrappers
namespace char_db {

export template <typename T, typename CharT>
  concept checked_database_of = requires (std::vector<CharT> seq, char32_t code_point)
  {
    requires std::same_as<typename T::char_type, CharT>;

    typename T::decoding_error;
    typename T::encoding_error;

    { T::front_mblen (seq) }
      -> std::same_as<std::expected<std::size_t, typename T::decoding_error>>;

    { T::to_code_point (seq) }
      -> std::same_as<std::expected<char32_t, typename T::decoding_error>>;

    { T::code_unit_size (code_point) }
      -> std::same_as<std::expected<std::size_t, typename T::encoding_error>>;

    { T::code_point_on (code_point, std::span (seq)) }
      -> std::same_as<std::expected<void, typename T::encoding_error>>;

    { T::char_size (seq) }
      -> std::same_as<std::expected<std::size_t, typename T::decoding_error>>;

    { T::is_valid_char (seq) }
      -> std::same_as<std::expected<void, typename T::decoding_error>>;

    { T::starts_with_valid_char (seq) }
      -> std::same_as<std::expected<void, typename T::decoding_error>>;

    { T::validate_char_sequence (seq) }
      -> std::same_as<std::expected<void, typename T::decoding_error>>;

    { T::template code_point_to<std::vector<typename T::char_type>> (code_point) }
      -> std::same_as<std::expected<std::vector<typename T::char_type>, typename T::encoding_error>>;
  };

export enum class checked_policy : std::uint8_t
{
  nothing,
  error_code,
  formatted_string,
  structured,
};

export template <typename T, checked_policy Policy = checked_policy::nothing>
class checked;

export template <typename T>
  class checked<T, checked_policy::nothing>
  {
  public:
    using char_type = typename T::char_type;
    using decoding_error = std::monostate;
    using encoding_error = std::monostate;
  
  public:
    template <std::ranges::input_range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<std::size_t, decoding_error>
      front_mblen (R &&seq)
      {
        if (std::ranges::empty (seq))
          return std::unexpected (decoding_error ());
        
        if (auto const mblen = T::front_mblen (seq);
            0 != mblen)
          return std::expected<std::size_t, decoding_error> (mblen);
        else
          return std::unexpected (decoding_error ()); 
      }

    template <std::ranges::input_range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<char32_t, decoding_error>
      to_code_point (R &&seq)
      {
        if (auto const is_valid = is_valid_char (seq))
          return std::expected<char32_t, decoding_error> (T::to_code_point (seq));
        else
          return std::unexpected (is_valid.error ());
      }

    static constexpr std::expected<std::size_t, encoding_error>
    code_unit_size (char32_t const code_point)
    {
      if (auto const size = T::code_unit_size (code_point); size != 0)
        return std::expected<std::size_t, encoding_error> (size);
      else
        return std::unexpected (encoding_error ());
    }

    template <std::size_t Extent = std::dynamic_extent>
      static constexpr std::expected<void, encoding_error>
      code_point_on (char32_t const code_point, std::span<char_type, Extent> const dest)
      {
        if (auto const is_valid = checked<utf32, checked_policy::nothing>::is_valid_char (
                std::views::single (code_point) ))
          {
            if (auto const unit_size = code_unit_size (code_point))
              {
                if (std::ranges::size (dest) < unit_size.value ())
                  return std::unexpected (encoding_error ());
              }
            else
              return std::unexpected (unit_size.error ());

            T::code_point_on (code_point, dest);
            return std::expected<void, encoding_error> ();
          }
        else
          return std::unexpected (is_valid.error ());
      }

    template <std::ranges::input_range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<std::size_t, decoding_error>
      char_size (R &&seq)
      {
        if (std::ranges::empty (seq))
          return std::unexpected (decoding_error ());

        return std::expected<std::size_t, decoding_error> (T::char_size (seq));
      }

    template <std::ranges::input_range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<void, decoding_error>
      is_valid_char (R &&seq)
      {
        if (std::ranges::empty (seq))
          return std::unexpected (decoding_error ());

        return T::is_valid_char (seq)
               ? std::expected<void, decoding_error> ()
               : std::unexpected (decoding_error ());
      }

    template <std::ranges::input_range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<void, decoding_error>
      starts_with_valid_char (R &&seq)
      {
        if (std::ranges::empty (seq))
          return std::unexpected (decoding_error ());

        return T::starts_with_valid_char (seq)
               ? std::expected<void, decoding_error> ()
               : std::unexpected (decoding_error ());
      }

    template <std::ranges::input_range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<void, decoding_error>
      validate_char_sequence (R &&seq)
      {
        if (std::ranges::empty (seq))
          return std::unexpected (decoding_error ());

        return T::validate_char_sequence (seq)
               ? std::expected<void, decoding_error> ()
               : std::unexpected (decoding_error ());
      }

    template <std::ranges::range R>
    requires std::same_as<char_type, std::remove_cvref_t<std::ranges::range_value_t<R>>>
      static constexpr std::expected<R, encoding_error>
      code_point_to (char32_t const code_point)
      {
        if (auto const is_valid = checked<utf32, checked_policy::nothing>::is_valid_char (
                std::views::single (code_point) ))
          return T::template code_point_to<R> (code_point);
        else
          return std::unexpected (is_valid.error ());
      }
  };

// TODO: implement simple wrapper class template for existing databases
//       based on the "checked_database_of" philosophy. including:
//       [x] no extra information
//       [ ] simple error code
//       [ ] formatted string with the specific error reason
//       [ ] structured information (possibly error code + specific formatted reason in string + position, etc)

// TODO: tried_database_of concept; unified char_db exceptions

// TODO: simple wrapper class template for existing databases to use the 
//       standard char_db exceptions

} // namespace char_db