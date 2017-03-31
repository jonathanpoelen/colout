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
* \author    Jonathan Poelen <jonathan.poelen+falcon@gmail.com>
* \version   0.1
* \brief
*/

#include "colout/palette.hpp"
#include "colout/utils/string_view.hpp"
#include "colout/utils/range.hpp"

namespace
{
  struct c_string : colout::string_view
  {
    template<std::size_t n>
    constexpr c_string(char const (&s)[n])
    : string_view(s, n-1)
    {}
  };

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

  namespace palette_def
  {
    // `, ""` std::end(c_string) to string_view pointer is a error in clang:
    // cannot access base class of pointer past the end of object
    #define MK_PALETTE(name, ...) \
      constexpr c_string name##_[] {__VA_ARGS__, ""};
    # include "./decl_palette.hpp"
    #undef MK_PALETTE
  }

  template<std::size_t n>
  constexpr colout::PaletteRef
  def_to_ref(c_string const (&av)[n])
  {
    // ignore last string (see comment abose)
    return {av, av + n - 1};
  }

  struct Palette { c_string name; colout::PaletteRef palette; };
  constexpr Palette palettes[] = {
    #define MK_PALETTE(name, ...) {#name, def_to_ref(palette_def::name##_)},
    # include "./decl_palette.hpp"
    #undef MK_PALETTE
  };

  constexpr colout::PaletteRef empty_palette = {nullptr, nullptr};

  bool sv_eq(colout::string_view a, colout::string_view b)
  {
    if (a.size() != b.size()) {
      return false;
    }

    return std::equal(a.begin(), a.end(), b.begin());
  }

  constexpr colout::PaletteRef
  av(colout::string_view const & av)
  { return {&av, &av + 1}; }
}

colout::PaletteRef
colout::Palettes::get(colout::string_view sv, colout::Plan plan) const
{
  assert(sv.size());

  if (sv.size() == 1) {
    char c = sv[0];
    for (auto & o : styles) {
      if (o.c == c) {
        return av(o.cmd_);
      }
    }

    int i = int(plan);
    if ('A' <= c && c <= 'Z') {
      c = char(c - 'A' + 'a');
      i += 2;
    }

    for (auto & o : basic_colors) {
      if (o.c == c) {
        return av(o.cmds_[i]);
      }
    }

    if (reset_color.c == c) {
      return av(reset_color.cmd_);
    }
  }
  else {
    for (auto & o : styles) {
      if (sv_eq(o.name_, sv)) {
        return av(o.cmd_);
      }
    }

    int i = int(plan);
    char c = sv[0];
    if ('A' <= c && c <= 'Z') {
      c = char(c - 'A' + 'a');
      i += 2;
    }

    auto new_sv = sv.substr(1);

    for (auto & o : basic_colors) {
      if (c == o.name_[0] && sv_eq(o.name_.substr(1), new_sv)) {
        return av(o.cmds_[i]);
      }
    }

    for (auto & o : palettes) {
      if (sv_eq(o.name, new_sv)) {
        return o.palette;
      }
    }

    for (auto & o : light_colors) {
      for (auto & name : o.names_) {
        if (c == name[0] && sv_eq(name.substr(1), new_sv)) {
          return av(o.cmds_[i]);
        }
      }
    }

    for (auto & name : reset_color.name_) {
      if (sv_eq(name, sv)) {
        return av(reset_color.cmd_);
      }
    }
  }

  return empty_palette;
}
