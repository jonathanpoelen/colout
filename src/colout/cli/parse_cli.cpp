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

#include "colout/palette.hpp"
#include "colout/cli/parse_cli.hpp"
#include "colout/cli/parse_colors.hpp"
#include "colout/utils/range.hpp"
#include "colout/utils/overload.hpp"
#include "colout/cli/parse_options.hpp"

#include <falcon/cxx/cxx.hpp>

#include <iostream>
#include <fstream>
#include <limits>

#include <cerrno>
#include <cassert>

// #define DEBUG_TRACE
#include "colout/trace.hpp"


namespace colout { namespace cli {

using S = std::string;
//using F = ActiveFlags;
using CStr = char const*;
//using Param = ColoutParam;

inline int parse_cli(
  GlobalParam& globalParam,
  ColoutParam& coloutParam,
  int ac, char const* const* av
) {
  auto cli_flag = [](auto c, zstring zstr, zstring help, ActiveFlags f) {
    auto lbd = [f](CStr, ColoutParam& coloutParam){
      coloutParam.activated_flags |= f;
    };
    return CliParam<c, decltype(lbd), HasArgument::No>{zstr, help, lbd};
  };

  auto cli_optv = [](auto c, zstring zstr, zstring help, auto f) {
    return CliParam<c, decltype(f), HasArgument::Required>{zstr, help, f};
  };

  // TODO
  FALCON_DIAGNOSTIC_PUSH
  FALCON_DIAGNOSTIC_GCC_IGNORE("-Wunused-parameter")

  auto po = make_program_options(
    cli_flag(C<'h'>, "help", "", ActiveFlags::help),
    cli_flag(C<'w'>, "loop", "", ActiveFlags::loop_regex),
    cli_flag(C<'c'>, "loop-color", "", ActiveFlags::loop_color),
    cli_flag(C<'k'>, "keep-colormap", "", ActiveFlags::keep_color),
    cli_flag(C<'N'>, "lc-numeric", "", ActiveFlags::lc_numeric),
    cli_flag(C<'o'>, "use-locale", "", ActiveFlags::use_locale),
    cli_flag(C<'i'>, "ignore-case", "", ActiveFlags::ignore_case),
    // TODO restart group '( x , y , z , --restart )'
    // TODO break loop '( x , y , z , --break )'
    // TODO --next-line
    //
    // TODO . z~a , . none , . -za h: <<< abc -> [a]b[c] 'a' same as 'c'

    cli_optv(C<'P'>, "predefined-regex", "", [](CStr s, ColoutParam& coloutParam){
      if (0);
      #define M(n, S, d) else if (strcmp(S, s) == 0) \
        coloutParam.predefined_regex = PredefinedRegex::n;
      COLOUT_CLI_PREDEFINED_REGEX_VISITOR(M)
      #undef M
      else {
        // TODO user regex
        throw runtime_cli_error{S(s) + ": bad predefined-regex name"};
      }
      coloutParam.activated_flags |= ActiveFlags::regex;
      coloutParam.regex.clear();
    }),

    cli_optv(C<'l'>, "line", "", [](CStr, ColoutParam& coloutParam){
      coloutParam.activated_flags |=
        ActiveFlags::keep_color
      | ActiveFlags::regex
      ;
      coloutParam.predefined_regex = PredefinedRegex::all;
      coloutParam.regex.clear();
    }),

    cli_optv(C<'r'>, "no-reset-color", "", [](CStr, ColoutParam& coloutParam){
      coloutParam.activated_flags |= ActiveFlags::set_reset_color;
      coloutParam.esc.clear();
    }),
    cli_optv(C<'K'>, "normal-color", "", [](CStr s, ColoutParam& coloutParam){
      coloutParam.activated_flags |= ActiveFlags::set_reset_color;
      if (*s) {
        coloutParam.esc.clear();
      }
      else {
        ColorBuilder builder;
        parse_color(builder, {s, strlen(s)}, Plan::fg);
        coloutParam.esc = builder.get_color_and_clear().str_move();
      }
    }),

    cli_optv(C<'p'>, "regex", "", [](CStr s, ColoutParam& coloutParam){
      coloutParam.regex = s;
      coloutParam.activated_flags |= ActiveFlags::regex;
      coloutParam.predefined_regex = PredefinedRegex::none;
    }),

    cli_optv(C<'W'>, "n-loop", "", [](CStr s, ColoutParam& coloutParam){
      cli_set_int(coloutParam.n_loop, s);
      coloutParam.activated_flags |= ActiveFlags::set_n_loop;
    }),
    cli_optv(C<'T'>, "regex-type", "", [](CStr s, ColoutParam& coloutParam){
      #define elif(name, value) else if (0 == strcmp(s, #name)) \
      coloutParam.regex_type = std::regex_constants::value
      if (0) {}
      elif(js, ECMAScript);
      elif(ecmascript, ECMAScript);
      elif(ECMAScript, ECMAScript);
      elif(ext, extended);
      elif(extended, extended);
      elif(basic, basic);
      elif(awk, awk);
      elif(grep, grep);
      elif(egrep, egrep);
      #undef elif
      // TODO perl, string, ...
      else CLI_ERR_IF(1, "bad value");
    }),
    cli_optv(C<'t'>, "theme", "", [](CStr s, ColoutParam& coloutParam){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.activated_flags |= ActiveFlags::theme;
      coloutParam.go_label = s;
    }),
    cli_optv(C<'L'>, "list", "", [](CStr s, ColoutParam& coloutParam){
      // TODO
    }),
    cli_optv(C<'a'>, "label", "", [](CStr s, ColoutParam& coloutParam){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.label = s;
    }),
    cli_optv(C<'G'>, "goto-label", "", [](CStr s, ColoutParam& coloutParam){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.activated_flags &= ~ActiveFlags::jump_flags;
      coloutParam.activated_flags |= ActiveFlags::goto_label;
      coloutParam.go_label = s;
    }),
    cli_optv(C<'C'>, "call-label", "", [](CStr s, ColoutParam& coloutParam){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.activated_flags &= ~ActiveFlags::jump_flags;
      coloutParam.activated_flags |= ActiveFlags::call_label;
      coloutParam.go_label = s;
    }),
    cli_optv(C<'d'>, "delimiter", "", [](CStr s, GlobalParam& globalParam){
      CLI_ERR_IF(!*s, "is empty");
      CLI_ERR_IF(s[1], "too long");
      globalParam.delim_style = *s;
    }),
    cli_optv(C<'O'>, "locale", "", [](CStr s, ColoutParam& coloutParam){
      if (*s) {
        coloutParam.locale = s;
      }
    }),
    cli_optv(C<'\0'>, "continue-from-last-color", "", [](CStr, ColoutParam& coloutParam){
      coloutParam.activated_flags |= ActiveFlags::continue_from_last_color;
    }),
    cli_optv(C<'\0'>, "end-color-mark", "", [](CStr s, ColoutParam& coloutParam){
      coloutParam.activated_flags |= ActiveFlags::end_color_mark;
    })
    // TODO tag, push, pop, gpush, spush, repush
    // TODO =, <, >, eq, lt, gt
  );

  FALCON_DIAGNOSTIC_POP

  struct
  {
    ColoutParam& coloutParam;
    GlobalParam& globalParam;

    operator ColoutParam& () const { return coloutParam; }
    operator GlobalParam& () const { return globalParam; }
  } ctx {
    coloutParam, globalParam
  };

  int optint = po.parse_command_line(ac, av, ctx);
  if (optint < 0 || bool(coloutParam.activated_flags & ActiveFlags::help)) {
    po.help();
    return 0;
  }
  return optint;
}

struct Labels
{
  using Pair = std::pair<std::reference_wrapper<std::string const>, int>;
  struct Less
  {
    bool operator()(Pair const & a, Pair const & b) const
    {
      return a.first.get() < b.first.get();
    }
  };

  Labels(std::vector<cli::ColoutParam> const & coloutParams)
  {
    for (std::size_t i = 0; i < coloutParams.size(); ++i) {
      std::string const & label = coloutParams[i].label;
      if (label.size()) {
        labels.emplace_back(label, int(i));
      }
    }

    std::sort(labels.begin(), labels.end(), Less{});

    auto cmp = [](auto& a, auto& b){ return a.first.get() == b.first.get(); };
    auto p = std::adjacent_find(labels.begin(), labels.end(), cmp);
    CLI_ERR_IF(
      labels.end() != p,
      "Duplicated label " + p->first.get()
    );
  }

  int find(std::string const & label) const
  {
    auto p = std::lower_bound(
      labels.begin(), labels.end(),
      Pair{label, 0}, Less{}
    );
    CLI_ERR_IF(
      p == labels.end() || p->first.get() != label,
      "Unknown label " + label
    );
    return p->second;
  }

private:
  std::vector<Pair> labels;
};

class ColoutParamAst;

struct Themes
{
  Themes(ColoutParamAst & ast)
  : ast_(ast)
  {}

  int get_id_or_load(std::string name);

private:
  std::vector<std::pair<std::string, int>> themes_;
  ColoutParamAst & ast_;
};

struct ColoutParamAst
{
  std::vector<ColoutParam> coloutParams;
  colout::ColorBuilder colorBuilder;
  colout::Palettes palettes;

private:
  std::vector<unsigned> sub_stack;

public:
  ColoutParam& new_element()
  {
    coloutParams.emplace_back();
    return coloutParams.back();
  }

  ColoutParam& current()
  {
    return coloutParams.back();
  }

  void open_group()
  {
    auto & opt = current();
    if (bool(opt.activated_flags & ActiveFlags::regex)) {
      opt.activated_flags |= ActiveFlags::next_is_sub;
      sub_stack.emplace_back(coloutParams.size());
    }
    else {
      opt.activated_flags |= ActiveFlags::start_group;
      sub_stack.emplace_back(coloutParams.size() - 1u);
    }
  }

  unsigned close_group()
  {
    if (sub_stack.empty()) {
      throw runtime_cli_error("mismatching ')'");
    }
    check_no_wait_next();
    auto & opt = current();
    unsigned iopen = sub_stack.back();
    opt.activated_flags |= ActiveFlags::is_closed;
    opt.bind_index = int(iopen);
    coloutParams[iopen].bind_index = int(coloutParams.size()) - 1u;
    sub_stack.pop_back();
    return iopen;
  }

  void check_no_wait_next()
  {
    constexpr auto f = ActiveFlags::next_is_alt | ActiveFlags::next_is_seq;
    if (bool(f & coloutParams.back().activated_flags)) {
      throw runtime_cli_error("| or , not followed by a pattern");
    }
  }

  void final()
  {
    if (!sub_stack.empty()) {
      throw runtime_cli_error("mismatching '('");
    }

    if (coloutParams.empty()) {
      return ;
    }

    check_no_wait_next();

    Labels labels{coloutParams};

    load_themes(labels);

    for (auto & param : coloutParams) {
      for (auto & color_param : param.colors) {
        if (auto const * label = mpark::get_if<ColorParam::LabelName>(&color_param.color)) {
          auto const id = ColorParam::LabelId(labels.find(label->name));
          switch (label->type) {
            case ColorParam::LabelName::Type::Optional:
              // TODO Optional is not implemented
            case ColorParam::LabelName::Type::Label:
              if (mpark::get_if<mpark::monostate>(&label->mode)) {
                color_param.color = id;
              }
              else {
                color_param.color = ColorParam::Colors{
                  get_colors_from_label_id(id), std::move(label->mode)};
              }
              break;
            case ColorParam::LabelName::Type::Reference:
              color_param.color = ColorParam::Colors{
                get_colors_from_label_id(id), mpark::monostate{}};
              break;
           }
        }
      }
    }

    // TODO optimize go_id (jump and call)
  }

private:
  void load_themes(Labels & labels)
  {
    Themes themes{*this};

    auto first = begin(coloutParams);
    auto last = end(coloutParams);
    auto load_theme = [&](std::string theme_name){
        auto const param_sz = coloutParams.size();
        // /!\ can invalidate first and last iterators
        auto const id = themes.get_id_or_load(std::move(theme_name));
        if (param_sz < coloutParams.size()) {
          labels = Labels{coloutParams}; // TODO PERF
          first = begin(coloutParams) + (last - first);
          last = end(coloutParams);
        }
        return id;
    };
    for (; first != last; ++first) {
      ColoutParam & param = *first;
      if (bool(param.activated_flags & ActiveFlags::theme)) {
        param.activated_flags &= ~ActiveFlags::theme;
        param.activated_flags |= ActiveFlags::call_label;
        param.go_id = load_theme(std::move(param.go_label));
      }
      else {
        if (param.go_label.size()) {
          param.go_id = labels.find(param.go_label);
        }
        // /!\ iterators may be invalidated by load_theme
        for (auto i : range(0u, param.colors.size())) {
          ColorParam & color_param = first->colors[i];
          assert(!mpark::get_if<ColorParam::LabelId>(&color_param.color));
          if (auto const * theme = mpark::get_if<ColorParam::ThemeName>(&color_param.color)) {
            first->colors[i].color = ColorParam::LabelId(load_theme(theme->name));
          }
        }
      }
    }
  }

  std::vector<Color> get_colors_from_label_id(ColorParam::LabelId id)
  {
    std::vector<cli::ColorParam> const* colors_ptr = nullptr;
    while (!colors_ptr) {
      auto& ref_param = coloutParams[std::size_t(id)];
      switch (ref_param.colors.size()) {
        case 0:
          if (ref_param.go_id < 0) {
            CLI_ERR_IF(
              !ref_param.colors.size(),
              "No color on label :" + ref_param.label
            );
          }
          id = ColorParam::LabelId(ref_param.go_id);
          break;
        case 1: {
          auto& color = ref_param.colors[0].color;
          if (auto* pid = mpark::get_if<ColorParam::LabelId>(&color)) {
            id = *pid;
          }
          else {
            colors_ptr = &ref_param.colors;
          }
        } break;
        default:
          colors_ptr = &ref_param.colors;
      }
    }

    std::vector<Color> colors;
    colors.reserve(colors_ptr->size());
    for (ColorParam const& color_param : *colors_ptr) {
      auto* pcolor = mpark::get_if<Color>(&color_param.color);
      CLI_ERR_IF(
        !pcolor,
        "invalid color on label :" + coloutParams[std::size_t(id)].label
      );
      colors.emplace_back(*pcolor);
    }
    return colors;
  }
};

void colout_parse_cli_impl(ColoutParamAst & ast, Args args);

/**
 * \return errno or 0
 */
template<class String>
int get_file_contents(String& s, const char * name)
{
  typedef typename String::value_type char_type;
  typedef typename String::traits_type traits_type;

  std::basic_filebuf<char_type, traits_type> buf;

  char_type c;
  buf.pubsetbuf(&c, 1);

  if (!buf.open(name, std::ios::in)) {
    return errno;
  }

  const std::streamsize sz = buf.in_avail();
  if (sz == std::streamsize(-1)) {
    return errno;
  }

  s.resize(std::size_t(sz));
  const std::streamsize n = buf.sgetn(&s[0], sz);
  return (sz != n) ? s.resize(std::size_t(n)), errno : 0;
}


struct String
{
  using value_type = char;
  using traits_type = std::char_traits<char>;

  void resize(std::size_t n)
  {
    if (!d) {
      d.reset(new char[n+1]);
      sz = n;
    }
  }
  char& operator[](std::size_t i)
  {
    return d[i];
  }

  std::size_t size() const { return sz; }
  char * data() const { return d.get(); }

private:
  std::unique_ptr<char[]> d;
  std::size_t sz = 0;
};

int Themes::get_id_or_load(std::string name)
{
  for (auto && p : themes_) {
    if (p.first == name) {
      return p.second;
    }
  }

  String s;
  CLI_ERR_IF(
    int err = get_file_contents(s, name.c_str()),
    name + ": " + strerror(err)
  );

  auto p = reinterpret_cast<unsigned char*>(s.data());
  auto e = p + s.size();
  std::vector<char const *> v;

  static constexpr bool t[255]{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1,
  };
  auto skip = [&p, e]{
    while (p != e && t[*p]) {
      ++p;
    }
  };
  auto rskip = [&p, e]{
    while (p != e && !t[*p]) {
      ++p;
    }
  };

  skip();
  while (p < e) {
    skip();
    v.emplace_back(reinterpret_cast<char const*>(p));
    rskip();
    *p = '\0';
    ++p;
  }

  int ret = int(ast_.coloutParams.size());
  colout_parse_cli_impl(ast_, {int(v.size()), v.data()});
  themes_.emplace_back(std::move(name), ret);
  return ret;
}

std::vector<ColoutParam> colout_parse_cli(int ac, char const* const* av)
{
  Args args{ac, av};
  args.next(); // ignore program name
  ColoutParamAst ast;
  colout_parse_cli_impl(ast, args);
  // TODO check option
  ast.final();
  return std::move(ast.coloutParams);
}

void colout_parse_cli_impl(ColoutParamAst & ast, Args args)
{
  GlobalParam globalParam;

  while (args.is_valid()) {
    ColoutParam& coloutParam = ast.new_element();
    int optint = parse_cli(globalParam, coloutParam, args.ac(), args.av());
    if (optint < 0) {
        std::cout << "error: " << optint << "\n";
      // TODO return error if optint < 0
      break;
    }
    args.next(optint);

    if (!args.is_valid()) {
      // TODO error, except label, regex, theme
      if (!bool(coloutParam.activated_flags & ActiveFlags::regex)) {
        std::cout << "error: " << optint << "\n";
        break;
      }
      parse_colors(
        ast.colorBuilder,
        coloutParam.colors,
        inout(args),
        ast.palettes
      );
      break;
    }

    auto s = args.current();
    if (globalParam.delim_open == s[0] && !s[1]) {
      ast.open_group();
      args.next();
      continue;
    }

    auto exec_sep = [&](char const * s)
    {
      TRACE("check sep: ", s);
      if (globalParam.delim_open == s[0] && !s[1]) {
        ast.open_group();
        args.next();
        return true;
      }

      if (globalParam.delim_close == s[0] && !s[1]) {
        unsigned i;
        do {
          i = ast.close_group();
          args.next();
        } while (args.is_valid()
          && globalParam.delim_close == args.current()[0] && !args.current()[1]
        );
        if (args.is_valid()) {
          s = args.current();
          if (globalParam.delim_or == s[0] && !s[1]) {
            ast.coloutParams[i].activated_flags |= ActiveFlags::next_is_alt;
            args.next();
          }
          else if (globalParam.delim_list == s[0] && !s[1]) {
            ast.coloutParams[i].activated_flags |= ActiveFlags::next_is_seq;
            args.next();
          }
          else if (globalParam.delim_or == s[0] && globalParam.delim_list == s[1] && !s[2]) {
            ast.coloutParams[i].activated_flags |= ActiveFlags::next_is_alt | ActiveFlags::next_is_seq;
            args.next();
          }
        }
        return true;
      }

      if (globalParam.delim_or == s[0] && !s[1]) {
        coloutParam.activated_flags |= ActiveFlags::next_is_alt;
        args.next();
        return true;
      }

      if (globalParam.delim_list == s[0] && !s[1]) {
        coloutParam.activated_flags |= ActiveFlags::next_is_seq;
        args.next();
        return true;
      }

      if (globalParam.delim_or == s[0] && globalParam.delim_list == s[1] && !s[2]) {
        coloutParam.activated_flags |= ActiveFlags::next_is_alt | ActiveFlags::next_is_seq;
        args.next();
        return true;
      }

      return false;
    };

    if (coloutParam.go_label.size() && exec_sep(s)) {
      continue;
    }

    if (!bool(coloutParam.activated_flags & ActiveFlags::regex)) {
      coloutParam.activated_flags |= ActiveFlags::regex;
      coloutParam.regex = s;
      args.next();
    }

    do {
      if (args.is_valid() && exec_sep(args.current())) {
        break;
      }
      parse_colors(
        ast.colorBuilder,
        coloutParam.colors,
        inout(args),
        ast.palettes
      );
    }
    while (args.is_valid());
  }
}

} }
