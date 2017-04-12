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

#include <falcon/cxx/cxx.hpp>

#include <iostream>
#include <fstream>
#include <limits>

#include <cstdlib>
#include <cerrno>
#include <cassert>


namespace colout { namespace cli {

inline unsigned strtou(const char* s, char** end, int base)
{
  auto x = strtoul(s, end, base);
  if (x > std::numeric_limits<unsigned>::max()) {
    errno = ERANGE;
  }
  return static_cast<uint>(x);
}

inline int strtoi(const char* s, char** end, int base)
{
  auto x = strtol(s, end, base);
  if (x < std::numeric_limits<unsigned>::min()) {
    errno = ERANGE;
  }
  if (x > std::numeric_limits<unsigned>::max()) {
    errno = ERANGE;
  }
  return static_cast<int>(x);
}

#define CLI_ERR_IF(x, msg) do { if (x) throw runtime_cli_error{msg}; } while (0)

using sint = int;
using slint = long;
using sllint = long long;
using uint = unsigned;
using ulint = unsigned long;
using ullint = unsigned long long;
using ldouble = long double;

template<class T> T ston(const char* s, char** end) = delete;
template<> int ston<int>(const char* s, char** end) { return strtoi(s, end, 10); }
template<> slint ston<slint>(const char* s, char** end) { return strtol(s, end, 10); }
template<> sllint ston<sllint>(const char* s, char** end) { return strtoll(s, end, 10); }
template<> uint ston<uint>(const char* s, char** end) { return strtou(s, end, 10); }
template<> ulint ston<ulint>(const char* s, char** end) { return strtoul(s, end, 10); }
template<> ullint ston<ullint>(const char* s, char** end) { return strtoull(s, end, 10); }
template<> double ston<double>(const char* s, char** end) { return strtod(s, end); }
template<> ldouble ston<ldouble>(const char* s, char** end) { return strtold(s, end); }

template<class T>
void cli_set_int(T & x, char const* s)
{
  char* end;
  x = ston<T>(s, &end);
  CLI_ERR_IF(errno == ERANGE, "out of range");
  CLI_ERR_IF(end == s || *end, "bad value");
}

template<class T>
void cli_set_int(T & x, char const*& s, char end_c)
{
  char* end;
  x = ston<T>(s, &end);
  CLI_ERR_IF(errno == ERANGE, "out of range");
  CLI_ERR_IF(end == s || *end != end_c, "bad value");
  s = end+1;
}

template<class F, bool HasValue>
struct CliParam
{
  constexpr static bool has_value = HasValue;
  char c;
  zstring s;
  zstring help;
  F f;
};


using S = std::string;
//using F = ActiveFlags;
using CStr = char const*;
//using Param = ColoutParam;

inline int parse_cli(
  GlobalParam& globalParam,
  ColoutParam& coloutParam,
  int ac, char const* const* av
) {
  auto cli_flag = [](char c, zstring zstr, zstring help, ActiveFlags f) {
    auto lbd = [f](ColoutParam& coloutParam, CStr){
      coloutParam.activated_flags |= f;
    };
    return CliParam<decltype(lbd), 0>{c, zstr, help, lbd};
  };

  auto cli_optv = [](char c, zstring zstr, zstring help, auto f) {
    return CliParam<decltype(f), 1>{c, zstr, help, f};
  };

  // TODO
  FALCON_DIAGNOSTIC_PUSH
  FALCON_DIAGNOSTIC_GCC_IGNORE("-Wunused-parameter")

  auto cli_options = [](auto... opts) {
    return [opts...](auto cli_f){
      return cli_f(opts...);
    };
  }(
    cli_flag('h', "help", "", ActiveFlags::help),
    cli_flag('w', "loop", "", ActiveFlags::loop_regex),
    cli_flag('c', "loop-color", "", ActiveFlags::loop_color),
    cli_flag('k', "keep-colormap", "", ActiveFlags::keep_color),
    cli_flag('N', "lc-numeric", "", ActiveFlags::lc_numeric),
    cli_flag('o', "use-locale", "", ActiveFlags::use_locale),
    cli_flag('h', "scale-hidden", "", ActiveFlags::hidden_scale),
    cli_flag('j', "log", "", ActiveFlags::scale | ActiveFlags::scale_log),
    cli_flag('e', "exp", "", ActiveFlags::scale | ActiveFlags::scale_exp),
    cli_flag('D', "div", "", ActiveFlags::scale | ActiveFlags::scale_div),
    cli_flag('i', "ignore-case", "", ActiveFlags::ignore_case),
    // TODO restart group '( x , y , z , --restart )'

    cli_optv('P', "predefined-regex", "", [](ColoutParam& coloutParam, CStr s){
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

    cli_optv('l', "line", "", [](ColoutParam& coloutParam, CStr){
      coloutParam.activated_flags |=
        ActiveFlags::keep_color
      | ActiveFlags::regex
      ;
      coloutParam.predefined_regex = PredefinedRegex::all;
      coloutParam.regex.clear();
    }),

    cli_optv('r', "no-reset-color", "", [](ColoutParam& coloutParam, CStr){
      coloutParam.activated_flags |= ActiveFlags::set_reset_color;
      coloutParam.esc.clear();
    }),
    cli_optv('K', "set-normal-color", "", [](ColoutParam& coloutParam, CStr s){
      coloutParam.activated_flags |= ActiveFlags::set_reset_color;
      if (*s) {
        coloutParam.esc.clear();
      }
      else {
        ColorBuilder builder;
        parse_color(builder, {s, strlen(s)});
        coloutParam.esc = builder.get_color_and_clear().str_move();
      }
    }),

    cli_optv('p', "regex", "", [](ColoutParam& coloutParam, CStr s){
      coloutParam.regex = s;
      coloutParam.activated_flags |= ActiveFlags::regex;
      coloutParam.predefined_regex = PredefinedRegex::none;
    }),

    cli_optv('s', "scale", "", [](ColoutParam& coloutParam, CStr s){
      // TODO auto-scale x or x,x or x+20,x-22
      cli_set_int(coloutParam.scale_min, s, ',');
      cli_set_int(coloutParam.scale_max, s);
      CLI_ERR_IF(
        coloutParam.scale_min >= coloutParam.scale_max,
        "min is greater than max"
      );
      coloutParam.activated_flags |= ActiveFlags::scale;
    }),
    cli_optv('u', "units", "", [](ColoutParam& coloutParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      CLI_ERR_IF(coloutParam.has_units(), "there are already units");
      coloutParam.activated_flags |= ActiveFlags::scale;
      coloutParam.units = s;
    }),
    cli_optv('U', "units-expr", "", [](ColoutParam& coloutParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      CLI_ERR_IF(coloutParam.has_units(), "there are already units");
      coloutParam.activated_flags |= ActiveFlags::scale;
      // TODO
    }),
    cli_optv('E', "coeff", "", [](ColoutParam& coloutParam, CStr s){
      cli_set_int(coloutParam.unit_coeff, s);
      coloutParam.activated_flags |= ActiveFlags::scale;
    }),
    cli_optv('g', "group", "", [](ColoutParam& coloutParam, CStr s){
      cli_set_int(coloutParam.scale_match, s);
      coloutParam.activated_flags |= ActiveFlags::scale;
    }),
    cli_optv('S', "select-unit", "", [](ColoutParam& coloutParam, CStr s){
      // TODO
      coloutParam.activated_flags |= ActiveFlags::scale;
    }),

    cli_optv('W', "n-loop", "", [](ColoutParam& coloutParam, CStr s){
      cli_set_int(coloutParam.n_loop, s);
      coloutParam.activated_flags |= ActiveFlags::set_n_loop;
    }),
    cli_optv('T', "regex-type", "", [](ColoutParam& coloutParam, CStr s){
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
    cli_optv('t', "theme", "", [](ColoutParam& coloutParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.activated_flags |= ActiveFlags::theme;
      coloutParam.go_label = s;
    }),
    cli_optv('L', "list", "", [](ColoutParam& coloutParam, CStr s){
      // TODO
    }),
    cli_optv('a', "label", "", [](ColoutParam& coloutParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.label = s;
    }),
    cli_optv('G', "goto-label", "", [](ColoutParam& coloutParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.activated_flags &= ~ActiveFlags::jump_flags;
      coloutParam.activated_flags |= ActiveFlags::goto_label;
      coloutParam.go_label = s;
    }),
    cli_optv('C', "call-label", "", [](ColoutParam& coloutParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      coloutParam.activated_flags &= ~ActiveFlags::jump_flags;
      coloutParam.activated_flags |= ActiveFlags::call_label;
      coloutParam.go_label = s;
    }),
    cli_optv('d', "delimiter", "", [](GlobalParam& globalParam, CStr s){
      CLI_ERR_IF(!*s, "is empty");
      CLI_ERR_IF(s[1], "too long");
      globalParam.delim_style = *s;
    }),
    cli_optv('O', "locale", "", [](ColoutParam& coloutParam, CStr s){
      if (*s) {
        coloutParam.locale = s;
      }
    }),
    cli_optv('\0', "continue-from-last-color", "", [](ColoutParam& coloutParam, CStr){
      coloutParam.activated_flags |= ActiveFlags::continue_from_last_color;
    }),
    cli_optv('\0', "end-color-mark", "", [](ColoutParam& coloutParam, CStr s){
      coloutParam.activated_flags |= ActiveFlags::end_color_mark;
    })
    // TODO tag, push, pop, gpush, spush, repush
    // TODO =, <, >, eq, lt, gt
  );

  FALCON_DIAGNOSTIC_POP

  enum class State {
    unknown, fail, ok, ok_arg
  };

  auto for_each_options = [&cli_options](auto f){
    return cli_options([f](auto & ... opts){
      State st = State::unknown;
      FALCON_UNPACK(st == State::unknown ? void(st = f(opts)) : void());
      CLI_ERR_IF(st == State::unknown, "unknown option");
      return st;
    });
  };

  int optint = 0;
  char current_single_opt[2] {};
  zstring current_long_opt = nullptr;

  struct
  {
    ColoutParam& coloutParam;
    GlobalParam& globalParam;

    operator ColoutParam& () const { return coloutParam; }
    operator GlobalParam& () const { return globalParam; }
  } values_ctx {
    coloutParam, globalParam
  };

  try {
    for (; optint < ac; ++optint) {
      char const* arg = av[optint];
      if (arg[0] != '-') {
        break;
      }
      // long option
      if (arg[1] == '-') {
        if (arg[2] == '\0') {
          ++optint;
          break;
        }
        zstring zview = &arg[2];
        current_single_opt[0] = 0;
        current_long_opt = &arg[0];
        for_each_options(
          [&zview, &values_ctx, &optint, ac, av](auto const & opt){
            // TODO = support: --name=value
            if (opt.s == zview) {
              if (opt.has_value) {
                CLI_ERR_IF(
                  optint+1 == ac,
                  S("`--") + zview.c_str() + "` requires a value"
                );
                opt.f(values_ctx, av[optint+1]);
                ++optint;
                return State::ok;
              }
              else {
                opt.f(values_ctx, {});
                return State::ok;
              }
            }
            return State::unknown;
          }
        );
      }
      // short option
      else {
        current_long_opt = nullptr;
        current_single_opt[0] = 0;
        while (*++arg) {
          current_single_opt[0] = *arg;
          if (State::ok_arg == for_each_options(
            [&arg, &values_ctx, &optint, ac, av](auto const & opt){
              if (opt.c == *arg) {
                if (opt.has_value) {
                  if (arg[1]) {
                    opt.f(values_ctx, arg+1);
                    --optint;
                    return State::ok_arg;
                  }
                  CLI_ERR_IF(
                    optint+1 == ac,
                    S("-`") + opt.c + "` requires a value"
                  );
                  opt.f(values_ctx, av[optint+1]);
                  return State::ok_arg;
                }
                else {
                  opt.f(values_ctx, {});
                  return State::ok;
                }
              }
              return State::unknown;
            }
          )) {
            ++optint;
            break;
          }
        }
      }
    }
  }
  catch (runtime_cli_error const & err) {
    if (current_long_opt) {
      std::cerr << current_long_opt << ": " << err.what() << std::endl;
    }
    else {
      std::cerr << "-" << current_single_opt << ": " << err.what() << std::endl;
    }
    return -1;
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

    Themes themes{*this};

    Labels labels{coloutParams};
    auto first = begin(coloutParams);
    auto last = end(coloutParams);
    auto load_theme = [&](std::string && theme_name){
        auto const param_sz = coloutParams.size();
        // /!\ may invalidate first and last iterators
        auto const id = themes.get_id_or_load(std::move(theme_name));
        if (param_sz < coloutParams.size()) {
          labels = Labels{coloutParams}; // PERF
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
        // /!\ iterators may be invalidate by load_theme
        for (auto i : range(0u, param.colors.size())) {
          ColorParam & c = first->colors[i];
          if (c.is_label_category()) {
            c.id_label = labels.find(c.color.str());
          }
          else if (c.is_theme_category()) {
            auto id = load_theme(c.color.str_move());
            first->colors[i].id_label = id;
          }
        }
      }
    }

    // TODO optimize go_id (jump and call)
  }
};

struct Args : colout::Range<char const*const*>
{
  Args(int ac, char const*const* av)
  : first_(av)
  , last_(av+ac)
  {}

  char const* current() const noexcept { assert(is_valid()); return *first_; }
  bool is_valid() const noexcept { return first_ < last_; }
  void next() noexcept { assert(is_valid()); ++first_; }
  void next(int n) noexcept { assert(is_valid()); first_ += n; }

  int ac() const noexcept { return int(last_ - first_); }
  char const*const* av() const noexcept { return first_; }

private:
  char const*const* first_;
  char const*const* last_;
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
      std::cout << "help: " << "\n";
      // TODO help
      break;
    }
    args.next(optint);

    if (!args.is_valid()) {
      std::cout << "error: " << optint << "\n";
      // TODO error, except label, regex, theme
      // TODO default color
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

    // TODO default color

    for (; args.is_valid(); args.next()) {
      s = args.current();
      if (exec_sep(s)) {
        break;
      }

      parse_colors(
        ast.colorBuilder,
        coloutParam.colors,
        s,
        ast.palettes,
        globalParam.delim_style
      );
    }
  }
}

} }
