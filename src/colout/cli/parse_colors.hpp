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
    struct Modes
    {
      struct ColorMode
      {
        std::string reset_color;
        bool is_default_reset_color = true;
      };

      struct Cycle
      {
        ColorMode color_mode;
        bool is_recursive = false;
        bool is_looped = true;
      };

      struct Hash
      {
        ColorMode color_mode;
      };

      struct Random
      {
        ColorMode color_mode;
        uint64_t seed = 0;
      };

      // TODO scale by len
      struct Scale
      {
        ColorMode color_mode;
        string_view units;
        struct Unit
        {
          string_view name;
          int coeff;
        };
        std::vector<Unit> def_units;
        double scale_min = 0;
        double scale_max = 100;
        double unit_coeff = 1000;
        enum Mode { Uni, Log, Exp, Div } mode = Uni;
        Color overflow_color;
        Color underflow_color;
      };
    };

    // only with ColorVariant = std::vector<Color>. LabelName or ThemeName
    using ModeVariant = mpark::variant<
      mpark::monostate,
      Modes::Cycle,
      Modes::Hash,
      Modes::Random,
      Modes::Scale
    >;

    struct ThemeName { string_view name; };

    enum class LabelId : int {};

    struct LabelName
    {
      enum class Type { Label, Reference, Optional };
      string_view name;
      Type type;
      ModeVariant mode;
    };

    struct Colors
    {
      std::vector<Color> colors;
      ModeVariant mode;
    };

    using ColorVariant = mpark::variant<
      Color,
      Colors,
      LabelId,
      // Intermediate values
      LabelName,
      ThemeName
    >;

    ColorVariant color;

    template<class T>
    ColorParam(T&& x)
    : color(std::forward<T>(x))
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
