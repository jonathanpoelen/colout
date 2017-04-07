#define BOOST_TEST_MODULE ParseColor

#include "boost_unit_tests.hpp"

#include "colout/color.hpp"
#include "colout/palette.hpp"
#include "colout/cli/parse_cli.hpp"
#include "colout/cli/parse_colors.hpp"

#ifdef IN_IDE_PARSER
# define SV(s) s
#else
# define SV(s) s ## _sv
#endif
using colout::operator""_sv;

BOOST_AUTO_TEST_CASE(Color)
{
  colout::ColorBuilder builder;
  std::vector<colout::cli::ColorParam> colors;
  colout::Palettes palettes;

#define EQ(s)                         \
  BOOST_CHECK_EQ(1u, colors.size());  \
  BOOST_CHECK_EQ(colors[0].id_label, -1); \
  BOOST_CHECK_EQ(colors[0].color.str(), s); \
  colors.clear()

  colout::cli::parse_colors(builder, colors, SV("#f00,#00ff00"), palettes, ',');
  EQ("\033[38;2;240;0;0;38;2;0;255;0m");

  colout::cli::parse_colors(builder, colors, SV("#ffffff"), palettes, ',');
  EQ("\033[38;2;255;255;255m");

  colout::cli::parse_colors(builder, colors, SV("bg=#00f"), palettes, ',');
  EQ("\033[48;2;0;0;240m");

  colout::cli::parse_colors(builder, colors, SV("7"), palettes, ',');
  EQ("\033[7m");

  colout::cli::parse_colors(builder, colors, SV("@43"), palettes, ',');
  EQ("\033[38;5;43m");

  colout::cli::parse_colors(builder, colors, SV("bg=@23,32"), palettes, ',');
  EQ("\033[48;5;23;32m");

  colout::cli::parse_colors(builder, colors, SV("#00f,1;33"), palettes, ',');
  EQ("\033[38;2;0;0;240;1;33m");

  colout::cli::parse_colors(builder, colors, SV("34,20/44/188,1"), palettes, ',');
  EQ("\033[34;38;2;20;44;188;1m");

  colout::cli::parse_colors(builder, colors, SV("r"), palettes, ',');
  EQ("\033[31m");

  colout::cli::parse_colors(builder, colors, SV("red"), palettes, ',');
  EQ("\033[31m");

  colout::cli::parse_colors(builder, colors, SV("R"), palettes, ',');
  EQ("\033[1;31m");

  colout::cli::parse_colors(builder, colors, SV("Red"), palettes, ',');
  EQ("\033[1;31m");
}
