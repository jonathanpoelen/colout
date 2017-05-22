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
* \author    Jonathan Poelen <jonathan.poelen@gmail.com>
* \version   0.1
* \brief
*/

#include <falcon/cxx/cxx.hpp>

#include "colout/color.hpp"
#include "colout/palette.hpp"
#include "colout/cli/parse_cli.hpp"
#include "colout/cli/parse_options.hpp"
#include "colout/cli/parse_colors.hpp"
#include "colout/utils/c_string.hpp"
#include "colout/utils/get_lines.hpp"
#include "colout/utils/overload.hpp"

#include <cstring>


namespace colout { namespace cli {

namespace {

[[noreturn]] inline void throw_unknown_color(string_view color)
{
  throw runtime_cli_error{
    "unknown '" + std::string(color.begin(), color.end()) + "' color"
  };
}

[[noreturn]] inline void throw_bad_color_format(string_view color)
{
  throw runtime_cli_error{
    "bad color format for '" + std::string(color.begin(), color.end()) + '\''
  };
}

constexpr bool ishex_table[] {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr uint8_t hextoi_table[] {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14,
  15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void parse_hex_color(
  ColorBuilder & builder,
  string_view sv_color_
){
  auto throw_bad_format = [sv_color_]{throw_bad_color_format(sv_color_);};
  auto ishex = [](char c){ return ishex_table[uint8_t(c)]; };
  auto hex2i = [](char c){ return hextoi_table[uint8_t(c)]; };

  auto sv_color = sv_color_.substr(1);

  // #fff
  if (3 == sv_color.size()) {
    if (!(ishex(sv_color[0]) & ishex(sv_color[1]) & ishex(sv_color[2]))) {
      throw_bad_format();
    }
    builder.push_rgb444(
      hex2i(sv_color[0]),
      hex2i(sv_color[1]),
      hex2i(sv_color[2])
    );
  }
  // #ffffff
  else if (6 == sv_color.size()) {
    if (!(ishex(sv_color[0]) & ishex(sv_color[1]) & ishex(sv_color[2])
        & ishex(sv_color[3]) & ishex(sv_color[4]) & ishex(sv_color[5])
    )) {
      throw_bad_format();
    }
    builder.push_rgb888(
      uint8_t((hex2i(sv_color[0]) << 4) | hex2i(sv_color[1])),
      uint8_t((hex2i(sv_color[2]) << 4) | hex2i(sv_color[3])),
      uint8_t((hex2i(sv_color[4]) << 4) | hex2i(sv_color[5]))
    );
  }
  else {
    throw_bad_format();
  }
}

auto mk_stoi8(string_view sv_color)
{
  return [sv_color](char const * s, char ** end) {
    long v = strtol(s, end, 10);
    if (errno == ERANGE || v < 0 || v > 255) {
      throw_bad_color_format(sv_color);
    }
    return uint8_t(v);
  };
}

void parse_256_color(
  ColorBuilder & builder,
  string_view sv_color
){
  char * end;
  auto stoi8 = mk_stoi8(sv_color);
  builder.push_256color(stoi8(sv_color.data() + 1, &end));

  if (end != sv_color.end()) {
    throw_bad_color_format(sv_color);
  }
}

void parse_int_color(
  ColorBuilder & builder,
  string_view sv_color,
  Plan plan
){
  auto stoi8 = mk_stoi8(sv_color);

  char * end;
  char const * s = sv_color.data();

  auto r_or_i = stoi8(s, &end);

  // [0-255]/[0-255]/[0-255]
  if (end != sv_color.end() && *end == '/') {
    builder.push_plan(plan);
    s = end + 1;
    if (s == sv_color.end()) {
      throw_bad_color_format(sv_color);
    }
    auto g = stoi8(s, &end);
    if (end == sv_color.end() || *end != '/' || end+1 == sv_color.end()) {
      throw_bad_color_format(sv_color);
    }
    s = end + 1;
    auto b = stoi8(s, &end);
    builder.push_rgb888(r_or_i, g, b);
  }
  // [0-255][;...]
  else {
    while (end != sv_color.end() && *end == ';') {
      s = end + 1;
      stoi8(s, &end);
    }
    builder.push_palette(string_view{";", 1});
    builder.push_palette(sv_color);
  }

  if (end != sv_color.end()) {
    throw_bad_color_format(sv_color);
  }
}

// named color
struct NamedCmd
{
  c_string name_;
  char c;
  c_string cmd_;
};

struct Named2Cmd
{
  c_string name_[2];
  char c;
  c_string cmd_;
};

struct NamedCmd4
{
  c_string name_;
  char c;
  c_string cmds_[4];
};

struct Named3Cmd4
{
  c_string names_[3];
  c_string cmds_[4];
};

constexpr NamedCmd
styles[] {
  {"normal",    'n', ";0"},
  {"bold",      'o', ";1"},
  {"dim",       'd', ";2"},
  {"italic",    'i', ";3"},
  {"underline", 'u', ";4"},
  {"blink",     'l', ";5"},
  {"reverse",   'v', ";7"},
  {"hidden",    'h', ";8"},
};

constexpr Named2Cmd reset_color{
  {"default", "none"}, 'N', ";39"
};

#define COLOR(name, c, cmd1, cmd2) \
  {name, c, {";" #cmd1, ";" #cmd2, ";1;" #cmd1, ";1;" #cmd2}}

constexpr NamedCmd4
basic_colors[] {
  COLOR("black",   'k', 30, 40),
  COLOR("red",     'r', 31, 41),
  COLOR("green",   'g', 32, 42),
  COLOR("yellow",  'y', 33, 43),
  COLOR("blue",    'b', 34, 44),
  COLOR("magenta", 'm', 35, 45),
  COLOR("cyan",    'c', 36, 46),
  COLOR("gray",    'a', 37, 47),
  COLOR("white",   'w', 97, 107),
};

#undef COLOR

#define COLOR2(p1, p2, a, b, cmd1, cmd2) \
  {{p1 "_" a, p2 a, p2 b}, {";" #cmd1, ";" #cmd2, ";1;" #cmd1, ";1;" #cmd2}}

constexpr Named3Cmd4
light_colors[] {
  COLOR2("dark",  "d", "gray",    "a", 90, 100),
  COLOR2("light", "l", "red",     "r", 91, 101),
  COLOR2("light", "l", "green",   "g", 92, 102),
  COLOR2("light", "l", "yellow",  "y", 93, 103),
  COLOR2("light", "l", "blue",    "b", 94, 104),
  COLOR2("light", "l", "magenta", "m", 95, 105),
  COLOR2("light", "l", "cyan",    "c", 96, 106),
};

#undef COLOR2

string_view parse_named_color(
  string_view sv_color,
  Plan plan
){
  assert(sv_color.size());

  if (sv_color.size() == 1) {
    char c = sv_color[0];
    for (auto & o : styles) {
      if (o.c == c) {
        return o.cmd_;
      }
    }

    int i = int(plan);
    if ('A' <= c && c <= 'Z') {
      c = char(c - 'A' + 'a');
      i += 2;
    }

    for (auto & o : basic_colors) {
      if (o.c == c) {
        return o.cmds_[i];
      }
    }

    if (reset_color.c == c) {
      return reset_color.cmd_;
    }
  }
  else {
    for (auto & o : styles) {
      if (o.name_ == sv_color) {
        return o.cmd_;
      }
    }

    int i = int(plan);
    char c = sv_color[0];
    if ('A' <= c && c <= 'Z') {
      c = char(c - 'A' + 'a');
      i += 2;
    }

    auto new_sv = sv_color.substr(1);

    for (auto & o : basic_colors) {
      if (c == o.name_[0] && o.name_.substr(1) == new_sv) {
        return o.cmds_[i];
      }
    }

    for (auto & o : light_colors) {
      for (auto & name : o.names_) {
        if (c == name[0] && name.substr(1) == new_sv) {
          return o.cmds_[i];
        }
      }
    }

    for (auto & name : reset_color.name_) {
      if (name == sv_color) {
        return reset_color.cmd_;
      }
    }
  }

  return {};
}

bool parse_single_color(
  ColorBuilder & builder,
  string_view sv_color,
  Plan plan
){
  if (sv_color.empty()) {
    throw_unknown_color(sv_color);
  }

  // #fff, #ffffff
  if ('#' == sv_color[0]) {
    builder.push_plan(plan);
    parse_hex_color(builder, sv_color);
  }
  // @[0-255]
  else if ('@' == sv_color[0]) {
    builder.push_plan(plan);
    parse_256_color(builder, sv_color);
  }
  // [0-255][;...], [0-255]/[0-255]/[0-255]
  else if ('0' <= sv_color[0] && sv_color[0] <= '9') {
    parse_int_color(builder, sv_color, plan);
  }
  // named color
  else {
    auto color = parse_named_color(sv_color, plan);
    if (color.empty()) {
      return false;
    }
    builder.push_palette(color);
  }
  return true;
}

bool parse_color_mode_value_and_advance(
  ColorParam::Modes::Cycle & cycle,
  string_view & sv_color
){
  return false;
}

bool parse_color_mode_value_and_advance(
  ColorParam::Modes::Hash & hash,
  string_view & sv_color
){
  return false;
}

bool parse_color_mode_value_and_advance(
  ColorParam::Modes::Random & random,
  string_view & sv_color
){
  return false;
}

bool parse_color_mode_value_and_advance(
  ColorParam::Modes::Scale & scale,
  string_view & sv_color
){
  return false;
}

template<class T>
struct t_ { using type = T; };

/*
 * I = integer
 * F = floating point | I
 * OptColor = [-r,--no-reset-color] [-K,--normal-color color]
 * Mode(c, opts?) = c':'[color] | c'~' OptColor opts '~'[:][color]
 *
 * cycle  = Mode('c', [-c,--recursive] [-n,--no-loop])
 * hash   = Mode('h', [[-p] pattern])
 * random = Mode('a', [-s,--seed I])
 * scale  = Mode('s',
 *    [-U,--units units]
 *    [-m,--mode {uni|log|div|exp}]
 *    [-o,--overflow-color color]
 *    [-u,--underflow-color color]
 *    [-c,--coeff] F]
 *    [-C,--color-all]
 *    [-p,--pattern pattern]
 *    [[-s,--scale] {F|F-F}]
 * )
 *
 * TODO h^colors^colors_reset
 * TODO h~...^colors^colors
 * TODO t:$`[$&]$' NOTE replacement color $R, ${o,r}
 * TODO t^$`[$&]$'^$`[$&]$' -> T:replace:replace-for-next
 */
// [chas]:
ColorParam::ModeVariant parse_color_mode_and_advance(string_view & sv_color)
{
  ColorParam::ModeVariant mode;
  auto const tmp_sv_color = sv_color;

  auto parse_value = [&](auto t){
    sv_color.remove_prefix(1);

    if (sv_color.empty()) {
      sv_color = tmp_sv_color;
    }
    else {
      switch (sv_color.front()) {
        case ':':
        case '(':
          using T = typename decltype(t)::type;
          mode.emplace<T>();
          sv_color.remove_prefix(1);
          if (not sv_color.empty()) {
            parse_color_mode_value_and_advance(mpark::get<T>(mode), sv_color);
          }
          break;
        default:
          sv_color = tmp_sv_color;
          break;
      }
    }
  };

  switch (sv_color.front()) {
    case 'c': parse_value(t_<ColorParam::Modes::Cycle>{}); break;
    case 'h': parse_value(t_<ColorParam::Modes::Hash>{}); break;
    case 'a': parse_value(t_<ColorParam::Modes::Random>{}); break;
    case 's': parse_value(t_<ColorParam::Modes::Scale>{}); break;
    default: break;
  }

  return mode;
}

// fg= or bg=
Plan parse_color_plan_and_advance(string_view & sv_color, Plan plan){
  if (sv_color.size() > 3
    && sv_color[2] == '=' && sv_color[1] == 'g'
    && (sv_color[0] == 'b' || sv_color[0] == 'f')
  ) {
    plan = sv_color[0] == 'b' ? Plan::bg : Plan::fg;
    sv_color.remove_prefix(3);
  }
  return plan;
}

// '[' colors... ']' without '[' and ']'
std::vector<Color> parse_user_palette(
  ColorBuilder & builder,
  string_view const sv_palette,
  Palettes const & palettes,
  Plan const default_plan
){
  builder.save_states();
  std::vector<Color> user_palette;
  for (string_view const sv_color_list : get_lines(sv_palette, ' ')) {
    for (string_view sv_color : get_lines(sv_color_list, ',')) {
      Plan const plan = parse_color_plan_and_advance(sv_color, default_plan);
      if (parse_single_color(builder, sv_color, plan)) {
        user_palette.push_back(builder.get_color_and_clear());
      }
      else {
        PaletteRef palette = palettes.get(sv_color);
        if (palette.empty()) {
          throw_unknown_color(sv_color);
        }
        auto const saved_ctx = builder.get_saved_ctx();
        builder.push_plan(plan);
        builder.save_states();
        for (string_view const sv : palette) {
          builder.push_palette(sv);
          user_palette.emplace_back(builder.get_color_and_clear());
        }
        builder.set_saved_ctx(saved_ctx);
      }
    }
  }
  if (user_palette.empty()) {
    throw runtime_cli_error("empty palette");
  }
  return user_palette;
}

enum class GroupType { Palette, Other };

// palette, theme or label
GroupType parse_pack_color(
  ColorBuilder & builder,
  std::vector<ColorParam> & color_params,
  string_view const sv_color,
  Palettes const & palettes,
  Plan const plan,
  ColorParam::ModeVariant && mode
){
  PaletteRef palette = palettes.get(sv_color);
  if (palette.empty()) {
    using LabelName = ColorParam::LabelName;
    using Type = ColorParam::LabelName::Type;
    switch (sv_color[0]) {
    // label
    case ':' :
      color_params.emplace_back(LabelName{
        sv_color.substr(1), Type::Label, std::move(mode)});
      break;
    // optional label
    case '?' :
      color_params.emplace_back(LabelName{
        sv_color.substr(1), Type::Optional, std::move(mode)});
      break;
    // reference
    case '%' :
      if (mode.index()) {
        throw runtime_cli_error("a reference label cannot have a color mode");
      }
      color_params.emplace_back(LabelName{
        sv_color.substr(1), Type::Reference, mpark::monostate{}});
      break;
    // theme
    default:
      if (mode.index()) {
        throw runtime_cli_error("a theme cannot have a color mode");
      }
      color_params.emplace_back(ColorParam::ThemeName{sv_color});
    }
    return GroupType::Other;
  }

  builder.push_plan(plan);
  builder.save_states();

  if (mode.index()) {
    std::vector<Color> colors;
    for (string_view const sv : palette) {
      builder.push_palette(sv);
      colors.push_back(builder.get_color_and_clear());
    }
    color_params.emplace_back(ColorParam::Colors{
      std::move(colors), std::move(mode)});
  }
  else {
    for (string_view const sv : palette) {
      builder.push_palette(sv);
      color_params.emplace_back(builder.get_color_and_clear());
    }
  }
  return GroupType::Palette;
}

} // anonymous namespace

void parse_color(
  ColorBuilder & builder,
  string_view const sv_color_list,
  Plan const default_plan
){
  for (string_view sv_color : get_lines(sv_color_list, ',')) {
    Plan const plan = parse_color_plan_and_advance(sv_color, default_plan);
    if (!parse_single_color(builder, sv_color, plan)) {
      throw_unknown_color(sv_color);
    }
  }
}

void parse_colors(
  ColorBuilder & builder,
  std::vector<ColorParam> & color_params,
  string_view sv_color_list,
  Palettes const & palettes
){
  builder.reset();
  while (sv_color_list.size()) {
    ColorParam::ModeVariant mode = parse_color_mode_and_advance(sv_color_list);
    Plan const plan = parse_color_plan_and_advance(sv_color_list, Plan::fg);

    auto p = sv_color_list.begin();

    // user palette: [colors...]
    if (!sv_color_list.empty() && '[' == sv_color_list.front()) {
      p = std::find(p, sv_color_list.end()-1, ']');
      if (*p != ']') {
        throw runtime_cli_error("unmatched ']'");
      }
      color_params.emplace_back(ColorParam::Colors{
        parse_user_palette(
          builder, {sv_color_list.begin()+1, p}, palettes, plan),
        std::move(mode)
      });
      builder.reset();
      ++p;
    }
    else {
      p = std::find(p, sv_color_list.end(), ',');
      string_view sv_color{sv_color_list.begin(), p};
      bool const has_color_mode = bool(mode.index());
      if (!sv_color.empty() && parse_single_color(builder, sv_color, plan)) {
        if (has_color_mode) {
          throw runtime_cli_error("color mode with a single color");
        }
      }
      // palette, theme or label
      else {
        auto result_group = parse_pack_color(
          builder, color_params, sv_color, palettes, plan, std::move(mode)
        );
        builder.reset();
        if (GroupType::Palette != result_group || has_color_mode) {
          if (p != sv_color_list.end()) {
            throw runtime_cli_error(
              "label or theme cannot be followed by something"
            );
          }
        }
      }
    }

    if (p != sv_color_list.end() && *p == ',') {
      ++p;
    }
    sv_color_list = {p, sv_color_list.end()};
  }

  if (!builder.empty()) {
    color_params.emplace_back(builder.get_color_and_clear());
  }
}

} }
