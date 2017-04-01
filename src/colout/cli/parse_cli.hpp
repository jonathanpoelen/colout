/* The MIT License (MIT)

Copyright (c) 2016 jonathan poelen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**
* \author    Jonathan Poelen <jonathan.poelen+falcon@gmail.com>
* \version   0.1
* \brief
*/

#pragma once

#include "colout/color.hpp"
#include "colout/palette.hpp"

#include "colout/utils/enum_ops.hpp"
#include "colout/utils/zstring.hpp"

#include <vector>
#include <regex>


namespace colout {

namespace cli {
  enum class ActiveFlags
  {
    none,
    regex = (1 << 0),
    regex_int = (1 << 1),
    regex_float = (1 << 2),
    regex_flags = regex | regex_int | regex_float,
    loop_color = (1 << 3),
    keep_color = (1 << 4),
    no_reset = (1 << 5),
    set_reset = (1 << 6),
    set_n_loop = (1 << 7),
    loop_regex = (1 << 8),
    loop_flags = set_n_loop | loop_regex,
    lc_numeric = (1 << 9),
    use_locale = (1 << 10),
    hidden_scale = (1 << 11),
    scale_log = (1 << 12),
    scale_exp = (1 << 13),
    scale_div = (1 << 14),
    invert_loop = (1 << 15),
    ignore_case = (1 << 16),
    end_color_mark = (1 << 17),
    scale = (1 << 18),
    line_mode = (1 << 19),
    help = (1 << 20),
    color_only = (1 << 21),
    next_is_alt = (1 << 22),
    next_is_seq = (1 << 23),
    next_is_sub = (1 << 24),
    start_group = (1 << 25),
    is_close = (1 << 26),
    continue_from_last_color = (1 << 27),
    call_label = (1 << 28),
    goto_label = (1 << 29),
    jump_flags = goto_label | call_label,
  };
}

template<>
struct is_enum_flags<cli::ActiveFlags> : std::true_type
{};

namespace cli
{
  struct Units {};

  struct GlobalParam
  {
    char delim_style = ',';
    char delim_list = ',';
    char delim_or = '|';
    char delim_open = '(';
    char delim_close = ')';
    // TODO encoding
  };

  struct ColoutParam
  {
    ActiveFlags activated_flags = ActiveFlags::none;
    zstring regex;
    zstring regex_prefix;
    zstring regex_suffix;
    std::regex_constants::syntax_option_type regex_type = std::regex_constants::ECMAScript;
    zstring label;
    zstring go_label;
    zstring units;
    zstring locale;
    int n_loop = -1;
    double scale_min = 0;
    double scale_max = 100;
    unsigned scale_match = 1;
    unsigned unit_coeff = 1000;
    std::vector<Units> def_units;
    std::vector<Color> colors;
    int bind_index = -1;

    bool has_units() {
      return !def_units.empty() || units;
    }
  };

  struct runtime_cli_error final : std::runtime_error
  {
    explicit runtime_cli_error(const char* msg)
    : std::runtime_error(msg)
    {}
    explicit runtime_cli_error(std::string&& msg)
    : std::runtime_error(std::move(msg))
    {}
    explicit runtime_cli_error(std::string const& msg)
    : std::runtime_error(msg)
    {}
  };

  int parse_cli(
    GlobalParam& coloutGlobalParam,
    ColoutParam& coloutParam,
    int ac, char const* const* av
  );

  struct ColoutParams
  {
    unsigned countValidId;
    // TODO encoding
    std::vector<ColoutParam> coloutParams;
  };

  std::vector<ColoutParam> colout_parse_cli(int ac, char const* const* av);

}

}
