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

#pragma once

#include <vector>
#include <cstring>

#include <mpark/variant.hpp>

#include "colout/color.hpp"

#include "colout/utils/string_view.hpp"


namespace colout
{

class Palettes;
class ColorBuilder;

namespace cli
{

  struct ColorParam
  {
    enum class LabelId : int {};
    struct ThemeName { string_view name; };
    struct LabelName { string_view name; };

    mpark::variant<
      Color,
      LabelId,
      ThemeName,
      LabelName,
      std::vector<Color>
    > color;

    enum Mode {
      no_mode,
      hash,
      cycle,
      scale,
      random,
    } mode;

    template<class T>
    ColorParam(T && x, Mode m)
    : color(std::forward<T>(x))
    , mode(m)
    {}
  };

  void parse_color(
    ColorBuilder & builder,
    string_view sv_color,
    Plan default_plan
  );

  void parse_colors(
    ColorBuilder & builder,
    std::vector<ColorParam> & colors,
    string_view rng,
    Palettes const & palettes
  );

}
}
