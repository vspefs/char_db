#include <char_db.hpp>

#include <string_view>
#include <print>
#include <cstdint>
#include <algorithm>
#include <ranges>

int
main ()
{
  using namespace std::literals::string_view_literals;

  constexpr auto seq = u8"👍 I'm a UTF-8 code unit sequence!! ❤❤"sv;
  constexpr auto code_points = U"👍 I'm a UTF-8 code unit sequence!! ❤❤"sv;

  for (auto view = seq | char_db::views::decoding<char_db::utf8> ();
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