#define BOOST_TEST_MODULE Color

#include "boost_unit_tests.hpp"

#include "colout/color.hpp"


BOOST_AUTO_TEST_CASE(Color)
{
  colout::ColorBuilder builder;

  BOOST_CHECK_EQ(true, builder.empty());
  BOOST_CHECK_EQ("", builder.get_color_and_clear().str());
  builder.push_plan(colout::Plan::fg);
  builder.save_states();
  builder.push_rgb888(120, 60, 22);
  BOOST_CHECK_EQ("\033[38;2;120;60;22m", builder.get_color_and_clear().str());
  builder.push_rgb444(15, 2, 13);
  BOOST_CHECK_EQ("\033[38;2;240;32;208m", builder.get_color_and_clear().str());
  builder.push_palette(";1;2");
  BOOST_CHECK_EQ("\033[38;1;2m", builder.get_color_and_clear().str());
  builder.push_style(1);
  BOOST_CHECK_EQ("\033[38;1m", builder.get_color_and_clear().str());
  builder.push_256color(47);
  BOOST_CHECK_EQ("\033[38;5;47m", builder.get_color_and_clear().str());
  builder.reset();
  builder.push_plan(colout::Plan::bg);
  builder.push_rgb888(120, 60, 22);
  builder.push_palette(";1;2");
  BOOST_CHECK_EQ("\033[48;2;120;60;22;1;2m", builder.get_color_and_clear().str());
}
