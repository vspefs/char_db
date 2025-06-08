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

#include <char_db.hh>

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