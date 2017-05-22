#define BOOST_TEST_MODULE ParseColor

#include "boost_unit_tests.hpp"

#include "colout/color.hpp"
#include "colout/palette.hpp"
#include "colout/cli/parse_cli.hpp"
#include "colout/cli/parse_colors.hpp"
#include "colout/cli/runtime_cli_error.hpp"
#include "colout/utils/c_string.hpp"

#define SV(s) colout::c_string(s)

BOOST_AUTO_TEST_CASE(Color)
{
  using ColorParam = colout::cli::ColorParam;

  using Cycle = colout::cli::ColorParam::Modes::Cycle;
  using Hash = colout::cli::ColorParam::Modes::Hash;
  using Random = colout::cli::ColorParam::Modes::Random;
  using Scale = colout::cli::ColorParam::Modes::Scale;

  using Color = colout::Color;
  using ColorList = colout::cli::ColorParam::Colors;
  using ThemeName = colout::cli::ColorParam::ThemeName;
  using LabelName = colout::cli::ColorParam::LabelName;

  colout::ColorBuilder builder;
  std::vector<ColorParam> colors;
  colout::Palettes palettes;

#define TEST_MODE_(n, s, ColorT, Mem, ColorV, Check2)              \
  colout::cli::parse_colors(builder, colors, SV(s), palettes);     \
  BOOST_CHECK_EQ(n, colors.size());                                \
  BOOST_CHECK_EQ(mpark::get<ColorT>(colors[0].color).Mem, ColorV); \
  Check2;                                                          \
  colors.clear()

#define TEST_PARSER(s, cmd) \
  TEST_MODE_(1u, s, Color, str(), cmd, void())

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

#define TEST_MODE_LABEL(s, label_name, Mode)     \
  TEST_MODE_(1u, s, LabelName, name, label_name, \
    BOOST_CHECK(mpark::holds_alternative<Mode>(  \
      mpark::get<LabelName>(colors[0].color).mode)));

  TEST_MODE_LABEL(":aaa", "aaa", mpark::monostate);
  TEST_MODE_LABEL("h::aaa", "aaa", Hash);
  TEST_MODE_LABEL("c::aaa", "aaa", Cycle);
  TEST_MODE_LABEL("s::aaa", "aaa", Scale);
  TEST_MODE_LABEL("a::aaa", "aaa", Random);

#define TEST_MODE_THEME(s, theme_name) \
  TEST_MODE_(1u, s, ThemeName, name, theme_name, void())

  TEST_MODE_THEME("aaa", "aaa");
  BOOST_CHECK_THROW(
    colout::cli::parse_colors(builder, colors, SV("h:aaa"), palettes),
    colout::cli::runtime_cli_error);

#define TEST_PALETTE(s, cmd) \
  TEST_MODE_(6u, s, Color, str(), cmd, void())

  TEST_PALETTE("rainbow", "\033[38;35m");
  TEST_PALETTE("o,rainbow", "\033[1;38;35m");

#define TEST_MODE_PALETTE(s, Mode, cmd, count_color)           \
  colout::cli::parse_colors(builder, colors, SV(s), palettes); \
  BOOST_CHECK_EQ(1u, colors.size());                           \
  {                                                            \
    auto & vcolors = mpark::get<ColorList>(colors[0].color);   \
    BOOST_CHECK(mpark::holds_alternative<Mode>(vcolors.mode)); \
    BOOST_CHECK_EQ(vcolors.colors[0].str(), cmd);              \
    BOOST_CHECK_EQ(vcolors.colors.size(), count_color);        \
  }                                                            \
  colors.clear()

  TEST_MODE_PALETTE("h:rainbow", Hash, "\033[38;35m", 6);
  TEST_MODE_PALETTE("h:[rainbow]", Hash, "\033[38;35m", 6);
  TEST_MODE_PALETTE("h:[rainbow r rainbow]", Hash, "\033[38;35m", 13);
  TEST_MODE_PALETTE("o,h:rainbow", Hash, "\033[1;38;35m", 6);
  TEST_MODE_PALETTE("o,h:[rainbow]", Hash, "\033[1;38;35m", 6);
  TEST_MODE_PALETTE("h:o,rainbow", Hash, "\033[1;38;35m", 6);
  TEST_MODE_PALETTE("o,h:[rainbow r rainbow]", Hash, "\033[1;38;35m", 13);
  TEST_MODE_PALETTE("[r]", mpark::monostate, "\033[31m", 1);
  TEST_MODE_PALETTE("c:[r]", Cycle, "\033[31m", 1);
  TEST_MODE_PALETTE("[r g b]", mpark::monostate, "\033[31m", 3);
  TEST_MODE_PALETTE("c:[r g b]", Cycle, "\033[31m", 3);
}
