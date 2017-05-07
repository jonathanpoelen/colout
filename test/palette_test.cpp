#define BOOST_TEST_MODULE Color

#include "boost_unit_tests.hpp"

#include "colout/palette.hpp"
#include "colout/utils/c_string.hpp"


BOOST_AUTO_TEST_CASE(Color)
{
  colout::Palettes palettes;

  using S = colout::c_string;

  auto palette1 = palettes.get(S("rainbow"));
  BOOST_CHECK_EQ(palette1.size(), 6);
  auto palette2 = palettes.get(S("r:rainbow"));
  BOOST_REQUIRE_EQ(palette2.size(), 6);
  BOOST_CHECK_NE(palette1.front(), palette2.front());
  BOOST_CHECK_EQ(palette1.front(), palette2.back());
}
