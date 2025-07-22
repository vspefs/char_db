module;
#include <climits>

export module vspefs.char_db : containers;

import : utils;
import std;

namespace char_db::containers {

template <std::size_t N>
  class succinct_bitset
  {
  public:
    constexpr succinct_bitset () = default;
    constexpr explicit succinct_bitset (std::from_range_t, utils::container_compatible_range<bool> auto &&);

    [[nodiscard]] consteval std::size_t size () const noexcept;
    [[nodiscard]] constexpr std::size_t count () const noexcept;
    [[nodiscard]] constexpr bool at (std::size_t) const noexcept;

    // Returns the number of bits equal to Value in [0, pos).
    template <bool Value = true>
    [[nodiscard]] constexpr std::size_t rank (std::size_t) const noexcept;

    // Returns the position of the (k+1)th bit equal to Value.
    template <bool Value = true>
    [[nodiscard]] constexpr std::size_t select (std::size_t) const noexcept;

  private:
    using uintword_t = std::uintptr_t;
    static constexpr std::size_t word_bit_size = CHAR_BIT * sizeof (uintword_t);
    static constexpr std::size_t l2_bit_size = word_bit_size;
    static constexpr std::size_t l1_bit_size = 64 * l2_bit_size;

    std::array<uintword_t, (N - 1) / word_bit_size + 1> bits_;
    std::array<std::size_t, (N - 1) / l1_bit_size + 1> l1_;
    std::array<std::uint16_t, (N - 1) / l2_bit_size + 1> l2_;
    std::size_t total_set_bits_ = 0;
  };

template <>
  class succinct_bitset<std::dynamic_extent>
  {
  public:
    constexpr succinct_bitset () = default;
    constexpr explicit succinct_bitset (std::from_range_t, utils::container_compatible_range<bool> auto &&);

    [[nodiscard]] constexpr std::size_t size () const noexcept;
    [[nodiscard]] constexpr std::size_t count () const noexcept;
    [[nodiscard]] constexpr bool at (std::size_t) const noexcept;

    // Returns the number of bits equal to Value in [0, pos).
    template <bool Value = true>
    [[nodiscard]] constexpr std::size_t rank (std::size_t) const noexcept;

    // Returns the position of the (k+1)th bit equal to Value.
    template <bool Value = true>
    [[nodiscard]] constexpr std::size_t select (std::size_t) const noexcept;

  private:
    using uintword_t = std::uintptr_t;
    static constexpr std::size_t word_bit_size = CHAR_BIT * sizeof (uintword_t);
    static constexpr std::size_t l2_bit_size = word_bit_size;
    static constexpr std::size_t l1_bit_size = 64 * l2_bit_size;

    std::size_t total_bits_ = 0;
    std::vector<uintword_t> bits_;
    std::vector<std::size_t> l1_;
    std::vector<std::uint16_t> l2_;
    std::size_t total_set_bits_ = 0;
  };

template <std::size_t N>
  constexpr succinct_bitset<N>::succinct_bitset(
      std::from_range_t, utils::container_compatible_range<bool> auto &&bits)
  : bits_ (), l1_ (), l2_ ()
  {
    std::size_t l1_idx = 0;
    std::size_t l2_idx = 0;
    std::size_t l1_sum = 0;
    std::size_t l2_sum = 0;
    uintword_t word = 0;
    std::size_t word_pos = 0;

    for (auto const [index, bit] : bits | utils::views::enumerate)
      {
        if (bit)
          {
            word |= (static_cast<uintword_t> (1) << (index % word_bit_size));
            ++l2_sum;
            ++l1_sum;
            ++total_set_bits_;
          }
        if ((index + 1) % word_bit_size == 0 || index + 1 == N)
          {
            bits_[word_pos++] = word;
            word = 0;
          }
        if ((index + 1) % l2_bit_size == 0 || index + 1 == N)
          {
            l2_[l2_idx++] = static_cast<std::uint16_t> (l2_sum);
            l2_sum = 0;
          }
        if ((index + 1) % l1_bit_size == 0 || index + 1 == N)
          {
            l1_[l1_idx++] = l1_sum;
            l1_sum = 0;
          }
      }
    // Zero-fill any remaining words if input is shorter than N
    while (word_pos < bits_.size())
      {
        bits_[word_pos++] = word;
        word = 0;
      }
    while (l2_idx < l2_.size())
      l2_[l2_idx++] = 0;
    while (l1_idx < l1_.size())
      l1_[l1_idx++] = 0;
  }

template <std::size_t N>
  [[nodiscard]] consteval std::size_t
  succinct_bitset<N>::size () const noexcept
  {
    return N;
  }

template <std::size_t N>
  [[nodiscard]] constexpr std::size_t
  succinct_bitset<N>::count () const noexcept
  {
    return total_set_bits_;
  }

template <std::size_t N>
  constexpr bool
  succinct_bitset<N>::at(std::size_t pos) const noexcept
  {
    if (pos >= N)
      return false;
    std::size_t word_idx = pos / word_bit_size;
    std::size_t bit_idx = pos % word_bit_size;
    return (bits_[word_idx] >> bit_idx) & 1;
  }

template <std::size_t N>
  template <bool Value>
    constexpr std::size_t succinct_bitset<N>::rank(std::size_t pos) const noexcept
    {
      pos = std::min(pos, N);
      std::size_t res = 0;
      std::size_t l1_blocks = pos / l1_bit_size;
      std::size_t l2_blocks = pos / l2_bit_size;
      std::size_t word_idx = pos / word_bit_size;
      std::size_t bit_idx = pos % word_bit_size;

      if constexpr (Value)
        {
          if (l1_blocks)
            res += l1_[l1_blocks - 1];
          if (l2_blocks)
            res += l2_[l2_blocks - 1];
          for (std::size_t i = l2_blocks * (l2_bit_size / word_bit_size); i < word_idx; ++i)
            res += std::popcount(bits_[i]);
          if (bit_idx)
            res += std::popcount(bits_[word_idx] & ((static_cast<uintword_t> (1) << bit_idx) - 1));
          return res;
        }
      else
        {
          return pos - rank<true>(pos);
        }
    }

template <std::size_t N>
  template <bool Value>
    constexpr std::size_t succinct_bitset<N>::select(std::size_t k) const noexcept
    {
      if constexpr (Value)
        {
          if (k >= total_set_bits_)
            return N;
        }
      else
        {
          if (k >= N - total_set_bits_)
            return N;
        }

      std::size_t l1_acc = 0, l1_idx = 0;
      // Find superblock
      while (l1_idx < l1_.size() &&
             ((Value ? l1_acc + l1_[l1_idx] : (l1_idx + 1) * l1_bit_size - (l1_acc + l1_[l1_idx])) <= k))
        {
          l1_acc += l1_[l1_idx];
          ++l1_idx;
        }
      std::size_t l2_base = l1_idx * (l1_bit_size / l2_bit_size);
      std::size_t l2_acc = l1_acc, l2_idx = l2_base;
      // Find block
      while (l2_idx < l2_.size() &&
             ((Value ? l2_acc + l2_[l2_idx] : (l2_idx + 1) * l2_bit_size - (l2_acc + l2_[l2_idx])) <= k))
        {
          l2_acc += l2_[l2_idx];
          ++l2_idx;
        }
      std::size_t word_base = l2_idx * (l2_bit_size / word_bit_size);
      std::size_t acc = l2_acc;
      // Find word
      std::size_t word_idx = word_base;
      for (; word_idx < bits_.size(); ++word_idx)
        {
          std::size_t pop = Value ? std::popcount(bits_[word_idx]) : word_bit_size - std::popcount(bits_[word_idx]);
          if (acc + pop > k)
            break;
          acc += pop;
        }
      // Find bit in word
      uintword_t w = bits_[word_idx];
      for (std::size_t b = 0; b < word_bit_size && word_idx * word_bit_size + b < N; ++b)
        {
          bool bit = (w >> b) & 1;
          if (bit == Value)
            {
              if (acc == k)
                return word_idx * word_bit_size + b;
              ++acc;
            }
        }
      return N;
    }



constexpr succinct_bitset<std::dynamic_extent>::succinct_bitset(
    std::from_range_t, utils::container_compatible_range<bool> auto &&bits)
: total_bits_ (std::ranges::size (bits)),
  bits_ ((total_bits_ - 1) / word_bit_size + 1, 0),
  l1_ ((total_bits_ - 1) / l1_bit_size + 1, 0),
  l2_ ((total_bits_ - 1) / l2_bit_size + 1, 0)
{
  std::size_t l1_idx = 0;
  std::size_t l2_idx = 0;
  std::size_t l1_sum = 0;
  std::size_t l2_sum = 0;
  uintword_t word = 0;
  std::size_t word_pos = 0;

  for (auto const [index, bit] : bits | utils::views::enumerate)
    {
      if (bit)
        {
          word |= (static_cast<uintword_t> (1) << (index % word_bit_size));
          ++l2_sum;
          ++l1_sum;
          ++total_set_bits_;
        }
      if ((index + 1) % word_bit_size == 0 || index + 1 == total_bits_)
        {
          bits_[word_pos++] = word;
          word = 0;
        }
      if ((index + 1) % l2_bit_size == 0 || index + 1 == total_bits_)
        {
          l2_[l2_idx++] = static_cast<std::uint16_t> (l2_sum);
          l2_sum = 0;
        }
      if ((index + 1) % l1_bit_size == 0 || index + 1 == total_bits_)
        {
          l1_[l1_idx++] = l1_sum;
          l1_sum = 0;
        }
    }
  // Zero-fill any remaining words if input is shorter than total_bits_
  while (word_pos < bits_.size())
    {
      bits_[word_pos++] = word;
      word = 0;
    }
  while (l2_idx < l2_.size())
    l2_[l2_idx++] = 0;
  while (l1_idx < l1_.size())
    l1_[l1_idx++] = 0;
}


[[nodiscard]] constexpr std::size_t
succinct_bitset<std::dynamic_extent>::size () const noexcept
{
  return total_bits_;
}

[[nodiscard]] constexpr std::size_t
succinct_bitset<std::dynamic_extent>::count () const noexcept
{
  return total_set_bits_;
}

constexpr bool
succinct_bitset<std::dynamic_extent>::at(std::size_t pos) const noexcept
{
  if (pos >= total_bits_)
    return false;
  std::size_t word_idx = pos / word_bit_size;
  std::size_t bit_idx = pos % word_bit_size;
  return (bits_[word_idx] >> bit_idx) & 1;
}


template <bool Value>
  constexpr std::size_t succinct_bitset<std::dynamic_extent>::rank(std::size_t pos) const noexcept
  {
    pos = std::min(pos, total_bits_);
    std::size_t res = 0;
    std::size_t l1_blocks = pos / l1_bit_size;
    std::size_t l2_blocks = pos / l2_bit_size;
    std::size_t word_idx = pos / word_bit_size;
    std::size_t bit_idx = pos % word_bit_size;

    if constexpr (Value)
      {
        if (l1_blocks)
          res += l1_[l1_blocks - 1];
        if (l2_blocks)
          res += l2_[l2_blocks - 1];
        for (std::size_t i = l2_blocks * (l2_bit_size / word_bit_size); i < word_idx; ++i)
          res += std::popcount(bits_[i]);
        if (bit_idx)
          res += std::popcount(bits_[word_idx] & ((static_cast<uintword_t> (1) << bit_idx) - 1));
        return res;
      }
    else
      {
        return pos - rank<true>(pos);
      }
  }

template <bool Value>
  constexpr std::size_t succinct_bitset<std::dynamic_extent>::select(std::size_t k) const noexcept
  {
    if constexpr (Value)
      {
        if (k >= total_set_bits_)
          return total_bits_;
      }
    else
      {
        if (k >= total_bits_ - total_set_bits_)
          return total_bits_;
      }

    std::size_t l1_acc = 0, l1_idx = 0;
    // Find superblock
    while (l1_idx < l1_.size() &&
           ((Value ? l1_acc + l1_[l1_idx] : (l1_idx + 1) * l1_bit_size - (l1_acc + l1_[l1_idx])) <= k))
      {
        l1_acc += l1_[l1_idx];
        ++l1_idx;
      }
    std::size_t l2_base = l1_idx * (l1_bit_size / l2_bit_size);
    std::size_t l2_acc = l1_acc, l2_idx = l2_base;
    // Find block
    while (l2_idx < l2_.size() &&
           ((Value ? l2_acc + l2_[l2_idx] : (l2_idx + 1) * l2_bit_size - (l2_acc + l2_[l2_idx])) <= k))
      {
        l2_acc += l2_[l2_idx];
        ++l2_idx;
      }
    std::size_t word_base = l2_idx * (l2_bit_size / word_bit_size);
    std::size_t acc = l2_acc;
    // Find word
    std::size_t word_idx = word_base;
    for (; word_idx < bits_.size(); ++word_idx)
      {
        std::size_t pop = Value ? std::popcount(bits_[word_idx]) : word_bit_size - std::popcount(bits_[word_idx]);
        if (acc + pop > k)
          break;
        acc += pop;
      }
    // Find bit in word
    uintword_t w = bits_[word_idx];
    for (std::size_t b = 0; b < word_bit_size && word_idx * word_bit_size + b < total_bits_; ++b)
      {
        bool bit = (w >> b) & 1;
        if (bit == Value)
          {
            if (acc == k)
              return word_idx * word_bit_size + b;
            ++acc;
          }
      }
    return total_bits_;
  }

} // namespace char_db::containers