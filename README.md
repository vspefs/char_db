# char_db (WIP)

A modern C++ header-only library for general encoding/decoding of characters, built around native language features
like `char8_t`, `char16_t`, `char32_t`, and ranges, with built-in support for UTF-8, UTF-16, and UTF-32.

The author is a lazy dumb ass and this library is still a work in progress, which leave vast room for contribution, bug
fix or improvement. For example, author hasn't even figure out the lowest language standard supported. See
[TODO](TODO.md) for a to-do list.

## Features

- Header-only, zero-dependency (except standard library)
- Built-in Unicode character validation and code point conversion for UTF-8, UTF-16, UTF-32
- Range-based `decoding` view for iterating subsequences that encodes an Unicode character
- Designed for constexpr and compile-time usage (though no such view has been implemented now)
- Official integration to a wide range of build systems (which isn't there yet)
- C++20 module support (can you believe I'm lazy enough to not have done this?)

## Usage

Include the main header in your project:

```cpp
#include <char_db.hh>
```

Example: Iterate over UTF-8 code points in a string

```cpp
#include <char_db.hh>
#include <string_view>
#include <print>

int
main ()
{
  using namespace std::literals;
  auto seq = u8"Hello, 🌍!"sv;
  for (auto subseq : seq | char_db::views::decoding<char_db::utf8> ())
    {
      auto cp = char_db::utf8::to_code_point (subseq);
      auto mblen = std::ranges::size (subseq);
      std::println ("U+{:04X}, using {} UTF-8 code units", static_cast<std::uint32_t>(cp), mblen);
    }
}
```

## Building & Installing

This project uses CMake:

```sh
cmake -B build
cmake --build build
cmake --install build
```

To build the example, enable the `BUILD_EXAMPLE` option:

```sh
cmake -B build -DBUILD_EXAMPLE=ON
cmake --build build
```

## (Current) API Overview

- `char_db::utf8`, `char_db::utf16`, `char_db::utf32`: Static interfaces for encoding/decoding and validation
- `char_db::views::decoding<Db>`: Range adaptor for decoding code unit sequences into code points

## Contributing

Just contribute, bro. Note that I use (my own version of) GNU Code Style. You can use whatever you want, because
I'll reformat them before merges. Sorry if you're not a fan of GNU Style. Everyone has their own kinks.

## License

AGPL License. See [LICENSE](LICENSE) for details.
