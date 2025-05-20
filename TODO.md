Views

- A `char_db::views::decoded` view that finds every valid subsequence at construction, so it can be used compile-time
- A `char_db::views::encoding` view that encodes code points
- A `char_db::views::encoded` view that encodes but compile-time friendly like the one mentioned above
- A `char_db::views::code_points` view that directly provides code points
- A `char_db::views::encoding_convert` view that converts between encodings

Coroutine (Generators)

- A `char_db::co::decoding` generator that leverages the standard coroutine and generator facility
- Other generators

Execution (Senders)

- A `char_db::exec::decoding` sender that leverages the standard execution facility
- Other senders

General

- Let the goddamn author learn C++ and ranges to improve overall code quality
- C++20 module support
- Other improvements