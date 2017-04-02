#include <falcon/cxx/cxx.hpp>

#include "colout/color.hpp"
#include "colout/cli/parse_cli.hpp"
#include "colout/cli/parse_colors.hpp"
#include "colout/utils/get_lines.hpp"

#include <cstring>


namespace colout { namespace cli {

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

inline void parse_hex_color(
  ColorBuilder & builder,
  string_view color_
) {
  auto throw_bad_format = [color_]{throw_bad_color_format(color_);};
  auto ishex = [](char c){ return ishex_table[uint8_t(c)]; };
  auto hex2i = [](char c){ return hextoi_table[uint8_t(c)]; };

  auto color = color_.substr(1);

  // #fff
  if (3 == color.size()) {
    if (!(ishex(color[0]) & ishex(color[1]) & ishex(color[2]))) {
      throw_bad_format();
    }
    builder.push_rgb444(
      hex2i(color[0]),
      hex2i(color[1]),
      hex2i(color[2])
    );
  }
  // #ffffff
  else if (6 == color.size()) {
    if (!(ishex(color[0]) & ishex(color[1]) & ishex(color[2])
        & ishex(color[3]) & ishex(color[4]) & ishex(color[5])
    )) {
      throw_bad_format();
    }
    builder.push_rgb888(
      uint8_t((hex2i(color[0]) << 4) | hex2i(color[1])),
      uint8_t((hex2i(color[2]) << 4) | hex2i(color[3])),
      uint8_t((hex2i(color[4]) << 4) | hex2i(color[5]))
    );
  }
  else {
    throw_bad_format();
  }
}

inline auto mk_stoi8(string_view color) {
  return [color](char const * s, char ** end) {
    long v = strtol(s, end, 10);
    if (errno == ERANGE || v < 0 || v > 255) {
      throw_bad_color_format(color);
    }
    return uint8_t(v);
  };
}

inline void parse_256_color(
  ColorBuilder & builder,
  string_view color
) {
  char * end;
  auto stoi8 = mk_stoi8(color);
  builder.push_256color(stoi8(color.data() + 1, &end));

  if (end != color.end()) {
    throw_bad_color_format(color);
  }
}

inline void parse_int_color(
  ColorBuilder & builder,
  string_view color,
  Plan plan
) {
  auto stoi8 = mk_stoi8(color);

  char * end;
  char const * s = color.data();

  auto r_or_i = stoi8(s, &end);

  // [0-255]/[0-255]/[0-255]
  if (end != color.end() && *end == '/') {
    builder.push_plan(plan);
    s = end + 1;
    if (s == color.end()) {
      throw_bad_color_format(color);
    }
    auto g = stoi8(s, &end);
    if (end == color.end() || *end != '/' || end+1 == color.end()) {
      throw_bad_color_format(color);
    }
    s = end + 1;
    auto b = stoi8(s, &end);
    builder.push_rgb888(r_or_i, g, b);
  }
  // [0-255][;...]
  else {
    while (end != color.end() && *end == ';') {
      s = end + 1;
      stoi8(s, &end);
    }
    builder.push_palette(string_view{";", 1});
    builder.push_palette(color);
  }

  if (end != color.end()) {
    throw_bad_color_format(color);
  }
}

enum ParsePalette { Yes, No };

template<ParsePalette parse_palette>
void parse_color(
  ColorBuilder & builder,
  string_view color,
  std::vector<Color> & colors,
  Palettes const & palettes
) {
  // fg= or bg=
  Plan plan = Plan::fg;
  if (color.size() > 3
    && color[2] == '=' && color[1] == 'g'
    && (color[0] == 'b' || color[0] == 'f')
  ) {
    plan = color[0] == 'b' ? Plan::bg : Plan::fg;
    color.remove_prefix(3);
  }

  if (color.empty()) {
    throw_unknown_color(color);
  }

  // #fff, #ffffff
  if ('#' == color[0]) {
    builder.push_plan(plan);
    parse_hex_color(builder, color);
  }
  // @[0-255]
  else if ('@' == color[0]) {
    builder.push_plan(plan);
    parse_256_color(builder, color);
  }
  // [0-255][;...], [0-255]/[0-255]/[0-255]
  else if ('0' <= color[0] && color[0] <= '9') {
    parse_int_color(builder, color, plan);
  }
  // palette color
  else if (parse_palette == ParsePalette::Yes) {
    PaletteRef palette = palettes.get(color, plan);
    if (palette.empty()) {
      throw_unknown_color(color);
    }

    if (palette.size() == 1) {
      builder.push_palette(palette[0]);
    }
    else {
      builder.push_plan(plan);
      builder.save_states();
      for (string_view sv : palette) {
        builder.push_palette(sv);
        colors.push_back(builder.get_color_and_clear());
      }
    }
  }
  else {
    throw_unknown_color(color);
  }
}

void parse_colors(
  ColorBuilder & builder,
  std::vector<Color> & colors,
  string_view rng,
  Palettes const & palettes,
  char delimiter
) {
  builder.reset();
  for (string_view color : get_lines(rng, delimiter))
  {
    parse_color<ParsePalette::Yes>(builder, color, colors, palettes);
  }

  if (!builder.empty()) {
    colors.push_back(builder.get_color_and_clear());
  }
}

void parse_color(ColorBuilder & builder, string_view color)
{
  std::vector<Color> colors;
  Palettes palettes;
  builder.reset();
  parse_color<ParsePalette::No>(builder, color, colors, palettes);
}

} }
