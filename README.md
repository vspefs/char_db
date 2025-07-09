# char_db (WIP)

A modern C++ module library for general encoding/decoding of characters, built around native language features
like `char8_t`, `char16_t`, `char32_t`, and ranges, with built-in support for UTF-8, UTF-16, and UTF-32.

The author is a lazy dumb ass and this library is still a work in progress, which leave vast room for contribution, bug
fix or improvement. See [TODO](TODO.md) for a to-do list.

## Features

- C++20 module, zero-dependency (except standard library)
- Built-in Unicode character validation and code point conversion for UTF-8, UTF-16, UTF-32
- Range-based views for iterating subsequences that encodes Unicode characters
- Designed for constexpr and compile-time usage (though no such view has been implemented now)
- (BIG CHANGE!) Sorry! CMake DSL is dogshit but as the library tries to look cool as possible by only providing itself
  in C++20 modules, CMake is again our only supported build system now!

## Usage

Your project must use exact C++23 standard and no additional compiler flag that will break CMI compatibility, because
CMade devs and C++ community haven't really come up with a good enough model for module distribution.

After installation, find the package and link against it in CMake:

```cmake
find_package (char_db REQUIRED)

# ...

target_link_libraries (
    your_awesome_target
    PRIVATE
        char_db::char_db
)
```

Then you can import the module in your source files:

```cpp
import vspefs.char_db;
```

Example: Iterate over UTF-8 code points in a string

```cpp
import vspefs.char_db;
import std;

int
main ()
{
  using namespace std::literals::string_view_literals;

  constexpr auto seq = u8"👍 I'm a UTF-8 code unit sequence!! ❤❤"sv;
  constexpr auto code_points = U"👍 I'm a UTF-8 code unit sequence!! ❤❤"sv;

  for (auto view = seq | char_db::views::decoding<char_db::utf8>;
       auto const [index, subseq] : view | std::views::enumerate)
    {
      std::ranges::for_each (subseq, [] (char8_t const code_unit)
        {
          std::print ("{:#04x} ", static_cast<std::uint8_t> (code_unit));
        });
      std::print ("-> {} ", static_cast<std::uint32_t> (char_db::utf8::to_code_point (subseq)));
      std::println ("{}", char_db::utf8::to_code_point (subseq) == code_points[index]);
    }
}
```

## Building & Installing

This project now only uses CMake:

```sh
cmake -B build
cmake --build build
cmake --install build
```

To specify the library build type:

```sh
# as a shared library
cmake -B build -DBUILD_SHARED_LIBS=ON
cmake --build build

# as a static library
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build
```

## (Current) API Overview

- `char_db::utf8`, `char_db::utf16`, `char_db::utf32`: Static interfaces for encoding/decoding and validation
- `char_db::views::decoding<Db>`: Range adaptor for decoding code unit sequences into code points
- `char_db::views::decoded<Db>`: Range adaptor for iterating decoded code unit sequences that represent valid Unicode code points

## Contributing

Just contribute, bro. Note that I use (my own version of) GNU Code Style. You can use whatever you want, because
I'll reformat them before merges. Sorry if you're not a fan of GNU Style. Everyone has their own kinks.

## License

AGPL License. See [LICENSE](LICENSE) for details.
