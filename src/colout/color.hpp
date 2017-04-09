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

#pragma once

#include "colout/utils/string_view.hpp"

#include <iosfwd>
#include <string>
#include <cstring>


namespace colout
{
  constexpr char const * i8_to_cmd_table[]{
    ";0",  ";1",  ";2",  ";3",  ";4",  ";5",  ";6",  ";7",  ";8",  ";9",
    ";10", ";11", ";12", ";13", ";14", ";15", ";16", ";17", ";18", ";19",
    ";20", ";21", ";22", ";23", ";24", ";25", ";26", ";27", ";28", ";29",
    ";30", ";31", ";32", ";33", ";34", ";35", ";36", ";37", ";38", ";39",
    ";40", ";41", ";42", ";43", ";44", ";45", ";46", ";47", ";48", ";49",
    ";50", ";51", ";52", ";53", ";54", ";55", ";56", ";57", ";58", ";59",
    ";60", ";61", ";62", ";63", ";64", ";65", ";66", ";67", ";68", ";69",
    ";70", ";71", ";72", ";73", ";74", ";75", ";76", ";77", ";78", ";79",
    ";80", ";81", ";82", ";83", ";84", ";85", ";86", ";87", ";88", ";89",
    ";90", ";91", ";92", ";93", ";94", ";95", ";96", ";97", ";98", ";99",
    ";100",";101",";102",";103",";104",";105",";106",";107",";108",";109",
    ";110",";111",";112",";113",";114",";115",";116",";117",";118",";119",
    ";120",";121",";122",";123",";124",";125",";126",";127",";128",";129",
    ";130",";131",";132",";133",";134",";135",";136",";137",";138",";139",
    ";140",";141",";142",";143",";144",";145",";146",";147",";148",";149",
    ";150",";151",";152",";153",";154",";155",";156",";157",";158",";159",
    ";160",";161",";162",";163",";164",";165",";166",";167",";168",";169",
    ";170",";171",";172",";173",";174",";175",";176",";177",";178",";179",
    ";180",";181",";182",";183",";184",";185",";186",";187",";188",";189",
    ";190",";191",";192",";193",";194",";195",";196",";197",";198",";199",
    ";200",";201",";202",";203",";204",";205",";206",";207",";208",";209",
    ";210",";211",";212",";213",";214",";215",";216",";217",";218",";219",
    ";220",";221",";222",";223",";224",";225",";226",";227",";228",";229",
    ";230",";231",";232",";233",";234",";235",";236",";237",";238",";239",
    ";240",";241",";242",";243",";244",";245",";246",";247",";248",";249",
    ";250",";251",";252",";253",";254",";255"
  };

  enum class Plan { fg, bg };

  struct Color
  {
    std::string cmd_;

    using const_iterator = std::string::const_iterator;

    const std::string & str() const noexcept { return cmd_; }
    std::string str_move() noexcept { return std::move(cmd_); }
  };

  template<class Ch, class Tr>
  std::basic_ostream<Ch, Tr>&
  operator<<(std::basic_ostream<Ch, Tr>& os, Color const& color)
  {
    for (char c : color.str()) {
      if (c == '\x1b') {
        os << "\\e";
      }
      else {
        os << c;
      }
    }
    return os;
  }

  struct ColorBuilder
  {
    ColorBuilder()
    {
      cmd_.reserve(20);
    }

    void push_plan(Plan plan)
    {
      static_assert(static_cast<int>(Plan::fg) == 0, "");
      static_assert(static_cast<int>(Plan::bg) == 1, "");
      static constexpr char const * cmd_plan[]{";38", ";48"};
      cmd_ += cmd_plan[static_cast<int>(plan)];
      plan_ = plan;
    }

    void push_rgb888(uint8_t r, uint8_t g, uint8_t b)
    {
      cmd_ += ";2";
      cmd_ += i8_to_cmd_table[r];
      cmd_ += i8_to_cmd_table[g];
      cmd_ += i8_to_cmd_table[b];
    }

    void push_rgb444(uint8_t r, uint8_t g, uint8_t b)
    {
      auto to8 = [](uint8_t c) { return uint8_t(c * 16); };
      push_rgb888(to8(r), to8(g), to8(b));
    }

    void push_256color(uint8_t i256)
    {
      cmd_ += ";5";
      cmd_ += i8_to_cmd_table[i256];
    }

    void push_style(uint8_t i)
    {
      cmd_ += ';';
      cmd_ += char(i + '0');
    }

    void push_palette(string_view av)
    {
      cmd_.append(av.begin(), av.end());
    }

    Color get_color_and_clear()
    {
      std::string cmd;
      if (!cmd_.empty()) {
        cmd.reserve(3u + cmd_.size());
        cmd = "\033[";
        cmd.append(cmd_.begin() + 1, cmd_.end());
        cmd += 'm';
      }
      Color ret{std::move(cmd)};
      cmd_.resize(save_pos_);
      return ret;
    }

    void save_states()
    {
      save_pos_ = cmd_.size();
      save_plan_ = plan_;
    }

    void reset()
    {
      cmd_.clear();
      save_pos_ = 0;
      plan_ = Plan::fg;
      save_plan_ = Plan::fg;
    }

    bool empty() const noexcept
    {
      return save_pos_ == cmd_.size();
    }

  private:
    std::string cmd_;
    std::size_t save_pos_ = 0;
    Plan plan_ = Plan::fg;
    Plan save_plan_ = Plan::fg;
  };
}
