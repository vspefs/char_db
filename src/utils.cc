module vspefs.char_db : utils;

import std;

namespace char_db::utils {

// class non_propagating_cache

template <typename T> requires std::is_object_v<T>
  class non_propagating_cache : public std::optional<T>
  {
  public:
    constexpr non_propagating_cache () noexcept = default;
    constexpr non_propagating_cache (non_propagating_cache const &) noexcept;
    constexpr non_propagating_cache (non_propagating_cache &&other) noexcept;

    constexpr non_propagating_cache &operator= (non_propagating_cache const &other) noexcept;
    constexpr non_propagating_cache &operator= (non_propagating_cache &&other) noexcept;

    template <typename I>
    constexpr T &emplace_deref (I const &i);
  };

template <typename T> requires std::is_object_v<T>
  constexpr
  non_propagating_cache<T>::non_propagating_cache (non_propagating_cache const &) noexcept
  {
  }

template <typename T> requires std::is_object_v<T>
  constexpr
  non_propagating_cache<T>::non_propagating_cache (non_propagating_cache &&other) noexcept
  {
    other.reset ();
  }

template <typename T> requires std::is_object_v<T>
  constexpr non_propagating_cache<T> &
  non_propagating_cache<T>::operator= (non_propagating_cache const &other) noexcept
  {
    if (std::addressof (other) != this)
      std::optional<T>::reset ();
    return *this;
  }

template <typename T> requires std::is_object_v<T>
  constexpr non_propagating_cache<T> &
  non_propagating_cache<T>::operator= (non_propagating_cache &&other) noexcept
  {
    std::optional<T>::reset ();
    other.reset ();
    return *this;
  }

template <typename T> requires std::is_object_v<T>
  template <typename I>
    constexpr T &
    non_propagating_cache<T>::emplace_deref (I const& i)
    {
      std::optional<T>::emplace (*i);
      return **this;
    }


// class movable_box

template <typename T> requires std::move_constructible<T> && std::is_object_v<T>
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

template <typename T> requires std::move_constructible<T> && std::is_object_v<T>
  constexpr
  movable_box<T>::movable_box () noexcept (std::is_nothrow_default_constructible_v<T>)
                                 requires std::default_initializable<T>
  : movable_box (std::in_place)
  {
  }

template <typename T> requires std::move_constructible<T> && std::is_object_v<T>
  constexpr movable_box<T> &
  movable_box<T>::operator= (movable_box const &other)
    noexcept (std::is_nothrow_copy_constructible_v<T>)
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

template <typename T> requires std::move_constructible<T> && std::is_object_v<T>
  constexpr movable_box<T> &
  movable_box<T>::operator= (movable_box<T> &&other)
    noexcept (std::is_nothrow_move_constructible_v<T>)
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


// concept container_compatible_range

template <typename R, typename T> concept container_compatible_range =
    std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;


// class enumerate_view & adaptor views::enumerate
//
// This is a workaround for the facts that:
//
// 1. libc++ has not implemented std::views::enumerate yet
// 2. clang++ can't compile GCC std module (at least on my computer)
// 3. To use clangd with modules, we need to build char_db using clang toolchain
//
// So basically, if we use clang with libstdc++, we can't use std module.
// But if we use clang with libc++, we can't use std::views::enumerate.
// If we don't use clang at all, we don't get language service.
//
// In this case, I just stole the libstdc++ implementation. char_db is
// licensed under the AGPL-3.0, so hopefully there won't be any problem.

template <typename Range> concept range_with_movable_reference =
    std::ranges::input_range<Range>
    && std::move_constructible<std::ranges::range_reference_t<Range>>
    && std::move_constructible<std::ranges::range_rvalue_reference_t<Range>>;

template<typename Range> concept simple_view = 
    std::ranges::view<Range>
    && std::ranges::range<Range const>
    && std::same_as<std::ranges::iterator_t<Range>, std::ranges::iterator_t<Range const>>
    && std::same_as<std::ranges::sentinel_t<Range>, std::ranges::sentinel_t<Range const>>;

template<bool Const, typename T>
using maybe_const_t = std::conditional_t<Const, T const, T>;

template <std::ranges::view V>
requires range_with_movable_reference<V>
  class enumerate_view : public std::ranges::view_interface<enumerate_view<V>>
  {
  public:
    template <bool Const> class iterator;
    template <bool Const> class sentinel;

  public:
    enumerate_view () requires std::default_initializable<V> = default;
    
    constexpr explicit
    enumerate_view (V base)
    : base_ (std::move (base))
    {
    }

    constexpr auto
    begin() requires (!simple_view<V>)
    {
      return iterator<false> (std::ranges::begin (base_), 0);
    }

    constexpr auto
    begin() const requires range_with_movable_reference<V const>
    {
      return iterator<true> (std::ranges::begin(base_), 0);
    }

    constexpr auto
    end() requires (!simple_view<V>)
    {
      if constexpr (std::ranges::common_range<V> && std::ranges::sized_range<V>)
        return iterator<false> (std::ranges::end (base_), std::ranges::distance (base_));
      else
        return sentinel<false> (std::ranges::end (base_));
    }

    constexpr auto
    end() const requires range_with_movable_reference<V const>
    {
      if constexpr (std::ranges::common_range<V const> && std::ranges::sized_range<V const>)
        return iterator<true> (std::ranges::end (base_), std::ranges::distance (base_));
      else
        return sentinel<true> (std::ranges::end (base_));
    }

    constexpr auto
    size() requires std::ranges::sized_range<V>
    {
      return std::ranges::size (base_);
    }

    constexpr auto
    size() const requires std::ranges::sized_range<V const>
    {
      return std::ranges::size (base_);
    }

    constexpr V
    base() const & requires std::copy_constructible<V>
    { 
      return base_;
    }

    constexpr V
    base() &&
    { 
      return std::move (base_);
    }

  private:
    V base_ = V ();
  };

template<typename Range>
  enumerate_view (Range &&) -> enumerate_view<std::ranges::views::all_t<Range>>;

template<std::ranges::view V>
requires range_with_movable_reference<V>
  template<bool Const>
    class enumerate_view<V>::iterator
    {
    private:
      using Base = maybe_const_t<Const, V>;
      static auto
      iter_concept_helper ()
      {
        if constexpr (std::ranges::random_access_range<Base>)
          return std::random_access_iterator_tag{};
        else if constexpr (std::ranges::bidirectional_range<Base>)
          return std::bidirectional_iterator_tag{};
        else if constexpr (std::ranges::forward_range<Base>)
          return std::forward_iterator_tag{};
        else
          return std::input_iterator_tag{};
      }
 
    public:
      using iterator_concept = decltype (iter_concept_helper ());
      using difference_type = std::ranges::range_difference_t<Base>;
      using value_type = std::tuple<difference_type, std::ranges::range_value_t<Base>>;

      friend enumerate_view;

    private:
      using reference_type = std::tuple<difference_type, std::ranges::range_reference_t<Base>>;

    public:
      iterator () requires std::default_initializable<std::ranges::iterator_t<Base>> = default;

      constexpr
      iterator (iterator<!Const> other)
        requires (Const && std::convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>)
      : current_ (std::move (other.current_)), pos_ (other.pos_)
      {
      }

      constexpr std::ranges::iterator_t<Base> const &
      base () const & noexcept
      {
        return current_;
      }

      constexpr std::ranges::iterator_t<Base>
      base () &&
      {
        return std::move (current_);
      }

      constexpr difference_type
      index () const noexcept
      {
        return pos_;
      }

      constexpr auto
      operator* () const
      {
        return reference_type (pos_, *current_);
      }

      constexpr iterator &
      operator++ ()
      {
        ++current_;
        ++pos_;
        return *this;
      }

      constexpr void
      operator++ (int)
      {
        ++*this;
      }

      constexpr iterator
      operator++ (int) requires std::ranges::forward_range<Base>
      {
        auto tmp = *this;
        ++*this;
        return tmp;
      }

      constexpr iterator &
      operator-- () requires std::ranges::bidirectional_range<Base>
      {
        --current_;
        --pos_;
        return *this;
      }

      constexpr iterator
      operator-- (int) requires std::ranges::bidirectional_range<Base>
      {
        auto tmp = *this;
        --*this;
        return tmp;
      }

      constexpr iterator &
      operator+= (difference_type n) requires std::ranges::random_access_range<Base>
      {
        current_ += n;
        pos_ += n;
        return *this;
      }

      constexpr iterator&
      operator-= (difference_type n) requires std::ranges::random_access_range<Base>
      {
        current_ -= n;
        pos_ -= n;
        return *this;
      }

      constexpr auto
      operator[] (difference_type n) const requires std::ranges::random_access_range<Base>
      {
        return reference_type (pos_ + n, current_[n]);
      }

    private:
      constexpr explicit
      iterator (std::ranges::iterator_t<Base> current, difference_type pos)
      : current_ (std::move (current)), pos_ (pos)
      {
      }

    private:
      std::ranges::iterator_t<Base> current_ = std::ranges::iterator_t<Base> ();
      difference_type pos_ = 0;

      friend constexpr bool
      operator== (iterator const &x, iterator const &y) noexcept
      {
        return x.pos_ == y.pos_;
      }

      friend constexpr std::strong_ordering
      operator<=> (iterator const &x, iterator const &y) noexcept
      {
        return x.pos_ <=> y.pos_;
      }

      friend constexpr iterator
      operator+ (iterator const &x, difference_type y)
        requires std::ranges::random_access_range<Base>
      {
        return (auto (x) += y);
      }

      friend constexpr iterator
      operator+ (difference_type x, iterator const &y)
        requires std::ranges::random_access_range<Base>
      {
        return auto (y) += x;
      }

      friend constexpr iterator
      operator-(iterator const &x, difference_type y)
        requires std::ranges::random_access_range<Base>
      {
        return auto (x) -= y;
      }

      friend constexpr difference_type
      operator-(iterator const &x, iterator const &y) noexcept
      {
        return x.pos_ - y.pos_;
      }

      friend constexpr auto
      iter_move(iterator const &it)
        noexcept (noexcept (std::ranges::iter_move (it.current_))
                  && std::is_nothrow_move_constructible_v<std::ranges::range_rvalue_reference_t<Base>> )
      {
        return std::tuple<difference_type, std::ranges::range_rvalue_reference_t<Base>> (
                   it.pos_, std::ranges::iter_move (it.current_) );
      }
    };

template<std::ranges::view V>
requires range_with_movable_reference<V>
  template<bool Const>
    class enumerate_view<V>::sentinel
    {
    private:
      using Base = maybe_const_t<Const, V>;

      friend enumerate_view;

    public:
      sentinel () = default;

      constexpr
      sentinel (sentinel<!Const> other)
        requires (Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>)
      : end_ (std::move (other.end_))
      {
      }

      constexpr std::ranges::sentinel_t<Base>
      base () const
      {
        return end_;
      }

      template<bool OtherConst>
      requires std::sentinel_for<std::ranges::sentinel_t<Base>,
                                 std::ranges::iterator_t<maybe_const_t<OtherConst, V>> >
        friend constexpr bool
        operator== (iterator<OtherConst> const &x, sentinel const &y)
        {
          return x.current_ == y.end_;
        }

      template<bool OtherConst>
      requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>,
                                       std::ranges::iterator_t<maybe_const_t<OtherConst, V>> >
        friend constexpr std::ranges::range_difference_t<maybe_const_t<OtherConst, V>>
        operator- (iterator<OtherConst> const &x, sentinel const &y)
        {
          return x.current_ - y.end_;
        }

      template<bool OtherConst>
      requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>,
                                       std::ranges::iterator_t<maybe_const_t<OtherConst, V>> >
        friend constexpr std::ranges::range_difference_t<maybe_const_t<OtherConst, V>>
        operator-(sentinel const &x, iterator<OtherConst> const &y)
        {
          return x.end_ - y.current_;
        }

    private:
      constexpr explicit
      sentinel (std::ranges::sentinel_t<Base> end)
      : end_ (std::move (end))
      {
      }

    private:
      std::ranges::sentinel_t<Base> end_ = {};
    };
namespace views {

template<typename T>
  concept can_enumerate_view = requires
  {
    enumerate_view<std::ranges::views::all_t<T>> (std::declval<T> ());
  };

struct enumerate_adaptor : public std::ranges::range_adaptor_closure<enumerate_adaptor>
{
  template <std::ranges::viewable_range Range>
  requires can_enumerate_view<Range>
    constexpr auto
    operator() [[nodiscard]] (Range &&r) const
    {
      return enumerate_view<std::ranges::views::all_t<Range>> (std::forward<Range> (r));
    }
};

inline constexpr enumerate_adaptor enumerate;

} // namespace char_db::utils::views

} // namespace char_db::utils