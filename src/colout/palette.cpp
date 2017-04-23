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
#include "colout/utils/c_string.hpp"
#include "colout/utils/range.hpp"

namespace
{
  namespace palette_def
  {
    // `, ""` std::end(c_string) to string_view pointer is a error in clang:
    // cannot access base class of pointer past the end of object
    #define MK_PALETTE(name, ...) \
      constexpr colout::c_string name##_[] {__VA_ARGS__, ""};
    # include "./decl_palette.hpp"
    #undef MK_PALETTE
  }

  template<std::size_t n>
  constexpr colout::PaletteRef
  def_to_ref(colout::c_string const (&av)[n])
  {
    // ignore last string (see comment above)
    return {av, av + n - 1};
  }

  struct Palette { colout::c_string name; colout::PaletteRef palette; };
  constexpr Palette palettes[] = {
    #define MK_PALETTE(name, ...) {#name, def_to_ref(palette_def::name##_)},
    # include "./decl_palette.hpp"
    #undef MK_PALETTE
  };

  constexpr colout::PaletteRef empty_palette = {nullptr, nullptr};
}

colout::PaletteRef
colout::Palettes::get(colout::string_view sv) const
{
  assert(sv.size());

  for (auto & o : palettes) {
    if (o.name == sv) {
      return o.palette;
    }
  }

  return empty_palette;
}
