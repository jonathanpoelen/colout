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
#include "colout/cli/parse_options.hpp"
#include "colout/utils/c_string.hpp"
#include "colout/utils/get_lines.hpp"
#include "colout/utils/overload.hpp"

// #define DEBUG_TRACE
#include "colout/trace.hpp"

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
  {"none",      'N', ";39"},
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

using CStr = char const*;

template<class Ctx, class... CliParams>
int parse_ctx(Ctx & ctx, InOut<Args> args, CliParams... cliParams)
{
  bool help = false;
  auto po = make_program_options(
    make_param(C<'h'>, "help", "", [&](CStr, Ctx&){ help = 1; }),
    std::move(cliParams)...
  );

  int optint = po.parse_command_line(args.value.ac(), args.value.av(), ctx);
  if (help) {
    po.help();
    return -2;
  }

  return optint;
}

int parse_color_mode_value_and_advance(
  ColorParam::Modes::Cycle & cycle,
  InOut<Args> args)
{
  using Cycle = ColorParam::Modes::Cycle;
  return parse_ctx(cycle, args,
    make_param(C<'r'>, "recursive", "",
      [&](CStr, Cycle & p){ p.is_recursive = 1; }),
    make_param(C<'n'>, "no-loop", "",
      [&](CStr, Cycle & p){ p.is_looped = 1; })
  );
}

int parse_color_mode_value_and_advance(
  ColorParam::Modes::Hash & hash,
  InOut<Args> args
){
  using Hash = ColorParam::Modes::Hash;
  return parse_ctx(hash, args,
    make_param(C<'p'>, "pattern", "",
      [&](CStr s, Hash & p){ p.sub_pattern = s; })
  );
}

int parse_color_mode_value_and_advance(
  ColorParam::Modes::Random & random,
  InOut<Args> args
){
  using Random = ColorParam::Modes::Random;
  return parse_ctx(random, args,
    make_param(C<'s'>, "seed", "",
      [&](CStr s, Random & p){ cli_set_int(p.seed, s); })
  );
}

int parse_color_mode_value_and_advance(
  ColorParam::Modes::Scale & scale,
  InOut<Args> args
){
  using Scale = ColorParam::Modes::Scale;
  return parse_ctx(scale, args,
    make_param(C<'U'>, "units", "",
      [&](CStr /*s*/, Scale & /*p*/){ /*TODO p.def_units*/; }),
    make_param(C<'m'>, "mode", "",
      [&](CStr s, Scale & p){
        if(0){}
        else if (!strcmp(s, "uni")) p.mode = Scale::Uni;
        else if (!strcmp(s, "log")) p.mode = Scale::Log;
        else if (!strcmp(s, "exp")) p.mode = Scale::Exp;
        else if (!strcmp(s, "div")) p.mode = Scale::Div;
        else throw runtime_cli_error("bad value");
      }),
    make_param(C<'o'>, "overflow-color", "",
      [&](CStr s, Scale & p){
        ColorBuilder builder;
        parse_color(builder, s, Plan::fg);
        p.overflow_color = builder.get_color_and_clear();
      }),
    make_param(C<'u'>, "underflow-color", "",
      [&](CStr s, Scale & p){
        ColorBuilder builder;
        parse_color(builder, s, Plan::fg);
        p.underflow_color = builder.get_color_and_clear();
      }),
    make_param(C<'b'>, "bounds-color", "",
      [&](CStr s, Scale & p){
        ColorBuilder builder;
        parse_color(builder, s, Plan::fg);
        p.overflow_color = builder.get_color_and_clear();
        p.underflow_color = p.overflow_color;
      }),
    make_param(C<'c'>, "coeff", "",
      [&](CStr s, Scale & p){ cli_set_int(p.unit_coeff, s); }),
    make_param(C<'p'>, "pattern", "",
      [&](CStr s, Scale & p){ p.sub_pattern = s; }),
    make_param(C<'s'>, "scale", "",
      [&](CStr s, Scale & p){
        // TODO auto-scale x or x,x or x+20,x-22
        cli_set_int(p.scale_min, s, ',');
        cli_set_int(p.scale_max, s);
        CLI_ERR_IF(
          p.scale_min >= p.scale_max,
          "min is greater than max"
         );
      })
  );
}

template<class T>
struct t_ { using type = T; };

struct ParseModeResult
{
  ColorParam::ModeVariant mode;
  string_view sv;
};

/*
 * I = integer
 * F = floating point | I
 * Mode(c, opts?) = c':'[color] | c'~' opts '~'[:][color] | c'~~'[:][color]
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
 * parser_color = cycle | hash | random | scale
 *
 * parser = [parser_color]['^'parser_color]
 *
 * TODO h~...^colors^colors
 * TODO t:$`[$&]$' NOTE replacement color $R, ${o,r}
 * TODO t^$`[$&]$'^$`[$&]$' -> T:replace:replace-for-next
 */
ParseModeResult parse_color_mode_and_advance(InOut<Args> args)
{
  ParseModeResult result;

  auto & mode = result.mode;
  auto & sv_color = result.sv;

  sv_color = args.value.current();
  args.value.next();

  auto parse_value = [&sv_color, &mode, &args](auto t){
    using T = typename decltype(t)::type;
    TRACE("possible mode: ", sv_color[0]);
    if (sv_color.size() != 1) {
      switch (sv_color[1]) {
        case ':':
          TRACE("simple mode");
          mode.emplace<T>();
          sv_color.remove_prefix(2);
          break;
        case '~':
          TRACE("mode with param");
          if (sv_color.size() == 2) {
            mode.emplace<T>();
            int optint = parse_color_mode_value_and_advance(mpark::get<T>(mode), args);
            CLI_ERR_IF(optint < 0, "bad syntax");
            args.value.next(optint);
            if (args.value.is_valid()) {
              sv_color = args.value.current();
              CLI_ERR_IF(!sv_color.empty() && sv_color[0] != '~', "bad syntax");
              sv_color.remove_prefix(1);
            }
            else {
              sv_color = {};
            }
          }
          else if (sv_color[2] == '~') {
            mode.emplace<T>();
            if (sv_color.size() == 3) {
              sv_color = args.value.is_valid() ? args.value.current() : nullptr;
            }
            else {
              sv_color.remove_prefix(3);
            }
          }
          break;
        default:
          break;
      }
    }
  };

  if (!sv_color.empty()) {
    switch (sv_color.front()) {
      case 'c': parse_value(t_<ColorParam::Modes::Cycle>{}); break;
      case 'h': parse_value(t_<ColorParam::Modes::Hash>{}); break;
      case 'a': parse_value(t_<ColorParam::Modes::Random>{}); break;
      case 's': parse_value(t_<ColorParam::Modes::Scale>{}); break;
      default: TRACE("no mode"); break;
    }
  }

  return result;
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

enum class GroupType { Palette, Theme, Label };

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
      TRACE("theme: ", sv_color);
      color_params.emplace_back(ColorParam::ThemeName{sv_color});
      return GroupType::Theme;
    }
    TRACE("label type: ", sv_color);
    return GroupType::Label;
  }

  builder.push_plan(plan);
  builder.save_states();

  if (mode.index()) {
    TRACE("palette in mode: ", sv_color);
    std::vector<Color> colors;
    for (string_view const sv : palette) {
      builder.push_palette(sv);
      colors.push_back(builder.get_color_and_clear());
    }
    color_params.emplace_back(ColorParam::Colors{
      std::move(colors), std::move(mode)});
  }
  else {
    TRACE("palette: ", sv_color);
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
  InOut<Args> args,
  Palettes const & palettes
){
  builder.reset();

  if (!args.value.is_valid()) {
    TRACE("empty arg, use default palette");
    for (string_view const sv : palettes.get_default_palette()) {
      builder.push_palette(sv);
      color_params.emplace_back(builder.get_color_and_clear());
    }
    return ;
  }

  TRACE("arg start: ", args.value.current());
  ParseModeResult parse_mode_result = parse_color_mode_and_advance(args);

  string_view sv_color_list = parse_mode_result.sv;
  TRACE("color list: ", sv_color_list);

  auto const beginning_count_color = color_params.size();

  while (sv_color_list.size()) {
    Plan const plan = parse_color_plan_and_advance(sv_color_list, Plan::fg);

    auto p = sv_color_list.begin();

    // user palette: [colors...]
    if (!sv_color_list.empty() && '[' == sv_color_list.front()) {
      p = std::find(p, sv_color_list.end()-1, ']');
      CLI_ERR_IF(*p != ']', "unmatched ']'");
      CLI_ERR_IF(p + 1 != sv_color_list.end(),
        "palette cannot be followed by something");
      color_params.emplace_back(ColorParam::Colors{
        parse_user_palette(
          builder, {sv_color_list.begin()+1, p}, palettes, plan),
        std::move(parse_mode_result.mode)
      });
      builder.reset();
      ++p;
    }
    else {
      p = std::find(p, sv_color_list.end(), ',');
      string_view sv_color{sv_color_list.begin(), p};
      if (sv_color.empty() || !parse_single_color(builder, sv_color, plan)) {
        auto result_group = parse_pack_color(
          builder, color_params, sv_color, palettes,
          plan, std::move(parse_mode_result.mode)
        );

        CLI_ERR_IF(GroupType::Theme == result_group
         && bool(parse_mode_result.mode.index()),
          "theme with a mode");

        if (GroupType::Palette != result_group) {
          CLI_ERR_IF(p != sv_color_list.end(),
            "label or theme cannot be followed by something");

          CLI_ERR_IF(!builder.empty(),
            "label or theme with color");
        }

        builder.reset();
      }
    }

    if (p != sv_color_list.end() && *p == ',') {
      ++p;
    }
    sv_color_list = {p, sv_color_list.end()};
    TRACE("next color list: ", sv_color_list);
  }

  if (!builder.empty()) {
    color_params.emplace_back(builder.get_color_and_clear());
  }
  else if (beginning_count_color == color_params.size()) {
    parse_pack_color(
      builder, color_params, string_view{}, palettes,
      Plan::fg, std::move(parse_mode_result.mode)
    );
  }
}

} }
