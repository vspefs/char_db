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

template <typename R, typename T> concept container_compatible_range =
    std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;

} // namespace char_db::utils