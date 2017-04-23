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

#include "colout/utils/enum_ops.hpp"
#include "colout/utils/zstring.hpp"
#include "colout/cli/parse_colors.hpp"

#include <vector>
#include <regex>


namespace colout {

class Color;

namespace cli {
  enum class ActiveFlags
  {
    none,
    regex = (1 << 0),
    set_reset_color = (1 << 1),
    continue_from_last_color = (1 << 2),
    loop_color = (1 << 3),
    keep_color = (1 << 4),
    call_label = (1 << 5),
    goto_label = (1 << 6),
    jump_flags = goto_label | call_label,
    set_n_loop = (1 << 7),
    loop_regex = (1 << 8),
    loop_flags = set_n_loop | loop_regex,
    lc_numeric = (1 << 9),
    use_locale = (1 << 10),
    hidden_scale = (1 << 11),
    scale_log = (1 << 12),
    scale_exp = (1 << 13),
    scale_div = (1 << 14),
    theme = (1 << 15),
    ignore_case = (1 << 16),
    end_color_mark = (1 << 17),
    scale = (1 << 18),
    auto_scale = (1 << 19),
    help = (1 << 20),
    color_only = (1 << 21),
    next_is_alt = (1 << 22),
    next_is_seq = (1 << 23),
    next_is_sub = (1 << 24),
    start_group = (1 << 25),
    is_closed = (1 << 26),
  };
}

template<>
struct is_enum_flags<cli::ActiveFlags> : std::true_type
{};

namespace cli
{
#define COLOUT_CLI_PREDEFINED_REGEX_VISITOR(F)                \
  F(all, "a", "^.*$")                                         \
  F(one_or_more, "A", "^.+$")                                 \
  F(integer, "i", "[-+]*[0-9]+")                              \
  F(float_point, "f", "[-+]*([0-9]+\\.[0-9]*|\\.[0-9]+)")     \
  F(float_comma, "f,", "[-+]*([0-9]+,[0-9]*|,[0-9]+)")        \
  F(scientific_point, "fs",                                   \
    "[-+]*([0-9]+\\.[0-9]*|\\.[0-9]+)(e[+-]?[0-9]+)?")        \
  F(scientific_comma, "fs,",                                  \
    "[-+]*([0-9]+,[0-9]*|,[0-9]+)(e[+-]?[0-9]+)?")            \
  F(unit_integer, "iU", "integer with unit (--unit)")         \
  F(unit_float_point, "fU",                                   \
    "floating point with unit (--unit)")                      \
  F(unit_float_comma, "f,U",                                  \
    "floating point with unit (--unit) and comma separator")  \
  F(unit_scientific_point, "fsU",                             \
    "scientific floating with unit (--unit)")                 \
  F(unit_scientific_comma, "fs,U",                            \
    "scientific floating unit (--unit) and comma separator")  \
  F(percent, "%", "[-+]*[0-9]+%")                             \
  F(percent_point, "%.", "[-+]*([0-9]+\\.[0-9]*|\\.[0-9]+)%") \
  F(percent_comma, "%,", "[-+]*([0-9]+,[0-9]*|,[0-9]+)%")     \
  F(open_brace, "{", "{...}")                                 \
  F(open_bracket, "[", "[...]")                               \
  F(open_parenthesis, "(", "(...)")                           \
  F(open_chevron, "<", "<...>")                               \
  F(close_brace, "}", "...}")                                 \
  F(close_bracket, "]", "...]")                               \
  F(close_parenthesis, ")", "...)")                           \
  F(close_chevron, ">", "...>")                               \
  F(until_comma, ",", "...,")                                 \
  F(simple_quoted, "q", "'[^'\]*(\.[^'\]*)*'")                \
  F(double_quoted, "Q", "(\"[^\"\\]*(\\.[^\"\\]*)*\"")        \
  F(quoted, "qq", "\"...\" or '...'")                         \
  F(simple_quoted_or_nor, "qs", "'...' or \\w")               \
  F(double_quoted_or_nor, "Qs", "\"...\" or \\w")             \
  F(quoted_or_nor, "qqs", "\"...\" or '...' or \\w")

  enum class PredefinedRegex
  {
    none,
#define M(n, s, d) n,
    COLOUT_CLI_PREDEFINED_REGEX_VISITOR(M)
#undef M
    user,
  };

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
    std::regex_constants::syntax_option_type regex_type = std::regex_constants::ECMAScript;
    PredefinedRegex predefined_regex = PredefinedRegex::none;
    std::string regex;
    std::string label;
    std::string go_label;
    std::string units;
    std::string locale;
    std::string esc;
    std::vector<Units> def_units;
    std::vector<ColorParam> colors;
    int n_loop = -1;
    double scale_min = 0;
    double scale_max = 100;
    unsigned scale_match = 1;
    unsigned unit_coeff = 1000;
    int bind_index = -1;
    int go_id = -1; // only if go_label is defined

    bool has_units() {
      return !def_units.empty() || !units.empty();
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

  std::vector<ColoutParam> colout_parse_cli(int ac, char const* const* av);

}

}
