#define BOOST_TEST_MODULE ParseColor

#include "boost_unit_tests.hpp"

#include "colout/color.hpp"
#include "colout/palette.hpp"
#include "colout/cli/parse_cli.hpp"
#include "colout/cli/parse_colors.hpp"
#include "colout/utils/c_string.hpp"

#define SV(s) colout::c_string(s)
using colout::operator""_sv;

BOOST_AUTO_TEST_CASE(Color)
{
  using ColorParam = colout::cli::ColorParam;
  colout::ColorBuilder builder;
  std::vector<ColorParam> colors;
  colout::Palettes palettes;

#define TEST_PARSER(s, cmd)                                              \
  colout::cli::parse_colors(builder, colors, SV(s), palettes);           \
  BOOST_CHECK_EQ(1u, colors.size());                                     \
  BOOST_CHECK_EQ(colors[0].mode, ColorParam::no_mode);                   \
  BOOST_CHECK_EQ(mpark::get<colout::Color>(colors[0].color).str(), cmd); \
  colors.clear()

  TEST_PARSER("#f00,#00ff00",   "\033[38;2;240;0;0;38;2;0;255;0m");
  TEST_PARSER("#ffffff",        "\033[38;2;255;255;255m");
  TEST_PARSER("bg=#00f",        "\033[48;2;0;0;240m");
  TEST_PARSER("7",              "\033[7m");
  TEST_PARSER("@43",            "\033[38;5;43m");
  TEST_PARSER("bg=@23,32",      "\033[48;5;23;32m");
  TEST_PARSER("#00f,1;33",      "\033[38;2;0;0;240;1;33m");
  TEST_PARSER("34,20/44/188,1", "\033[34;38;2;20;44;188;1m");
  TEST_PARSER("r",              "\033[31m");
  TEST_PARSER("red",            "\033[31m");
  TEST_PARSER("R",              "\033[1;31m");
  TEST_PARSER("Red",            "\033[1;31m");
  TEST_PARSER("o,r",            "\033[1;31m");
  TEST_PARSER("r,o",            "\033[31;1m");

#define TEST_MODE_(s, ColorType, Name, Mode)                                     \
  colout::cli::parse_colors(builder, colors, SV(s), palettes);                   \
  BOOST_CHECK_EQ(1u, colors.size());                                             \
  BOOST_CHECK_EQ(colors[0].mode, Mode);                                          \
  BOOST_CHECK_EQ(mpark::get<ColorParam::ColorType>(colors[0].color).name, Name); \
  colors.clear()

#define TEST_MODE_LABEL(s, label_name, Mode) \
  TEST_MODE_(s, LabelName, label_name, Mode)

  TEST_MODE_LABEL(":aaa", "aaa", ColorParam::no_mode);
  TEST_MODE_LABEL("h::aaa", "aaa", ColorParam::hash);
  TEST_MODE_LABEL("c::aaa", "aaa", ColorParam::cycle);
  TEST_MODE_LABEL("s::aaa", "aaa", ColorParam::scale);
  TEST_MODE_LABEL("a::aaa", "aaa", ColorParam::random);

#define TEST_MODE_THEME(s, theme_name, Mode) \
  TEST_MODE_(s, ThemeName, theme_name, Mode)

  TEST_MODE_THEME("aaa", "aaa", ColorParam::no_mode);
  BOOST_CHECK_THROW(
    colout::cli::parse_colors(builder, colors, SV("h:aaa"), palettes),
    colout::cli::runtime_cli_error);

#define TEST_PALETTE(s, cmd)                                             \
  colout::cli::parse_colors(builder, colors, SV(s), palettes);           \
  BOOST_CHECK_EQ(6u, colors.size());                                     \
  BOOST_CHECK_EQ(colors[0].mode, ColorParam::no_mode);                   \
  BOOST_CHECK_EQ(mpark::get<colout::Color>(colors[0].color).str(), cmd); \
  colors.clear()

  TEST_PALETTE("rainbow", "\033[38;35m");
  TEST_PALETTE("o,rainbow", "\033[1;38;35m");

#define TEST_MODE_PALETTE(s, Mode, cmd, count_color)                          \
  colout::cli::parse_colors(builder, colors, SV(s), palettes);                \
  BOOST_CHECK_EQ(1u, colors.size());                                          \
  BOOST_CHECK_EQ(colors[0].mode, Mode);                                       \
  {                                                                           \
    auto & vcolors = mpark::get<std::vector<colout::Color>>(colors[0].color); \
    BOOST_CHECK_EQ(vcolors[0].str(), cmd);                                    \
    BOOST_CHECK_EQ(vcolors.size(), count_color);                              \
  }                                                                           \
  colors.clear()

  TEST_MODE_PALETTE("h:rainbow", ColorParam::hash, "\033[38;35m", 6);
  TEST_MODE_PALETTE("h:[rainbow]", ColorParam::hash, "\033[38;35m", 6);
  TEST_MODE_PALETTE("h:[rainbow r rainbow]", ColorParam::hash, "\033[38;35m", 13);
  TEST_MODE_PALETTE("o,h:rainbow", ColorParam::hash, "\033[1;38;35m", 6);
  TEST_MODE_PALETTE("o,h:[rainbow]", ColorParam::hash, "\033[1;38;35m", 6);
  TEST_MODE_PALETTE("o,h:[rainbow r rainbow]", ColorParam::hash, "\033[1;38;35m", 13);
  TEST_MODE_PALETTE("[r]", ColorParam::no_mode, "\033[31m", 1);
  TEST_MODE_PALETTE("c:[r]", ColorParam::cycle, "\033[31m", 1);
  TEST_MODE_PALETTE("[r g b]", ColorParam::no_mode, "\033[31m", 3);
  TEST_MODE_PALETTE("c:[r g b]", ColorParam::cycle, "\033[31m", 3);
}
