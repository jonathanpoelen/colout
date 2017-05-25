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

#include "colout/utils/enum_ops.hpp"
#include "colout/utils/zstring.hpp"
#include "colout/cli/runtime_cli_error.hpp"

#include <falcon/cxx/cxx.hpp>

#include <iostream>
#include <string>
#include <cstdlib>


namespace colout {
namespace cli {

enum class HasArgument
{
  No,
  Required,
  Optional,
};

template<char c, class F, HasArgument HasValue>
struct CliParam
{
  constexpr static HasArgument has_argument() noexcept
  {
    return HasValue;
  }

  constexpr static char short_name() noexcept
  {
    return c;
  }

  constexpr zstring long_name() const noexcept
  {
    return s;
  }

  constexpr zstring help() const noexcept
  {
    return help_;
  }

  template<class... Args>
  void call(Args&&... args) const
  {
    f(static_cast<Args&&>(args)...);
  }

  zstring s;
  zstring help_;
  F f;
};

template<char c>
constexpr auto C = std::integral_constant<char, c>{};

template<HasArgument hasArg = HasArgument::No, class C, class F>
auto make_param(C c, zstring zstr, zstring help, F f)
{
  return CliParam<c, decltype(f), hasArg>{zstr, help, f};
}

template<HasArgument hasArg = HasArgument::No, class F>
auto make_param(zstring zstr, zstring help, F f)
{
  return CliParam<'\0', decltype(f), hasArg>{zstr, help, f};
}

template<class CliOptions>
struct ProgramOptions
{
  template<class... Args>
  int parse_command_line(int ac, char const* const* av, Args&&... args);

  void help();

  CliOptions mCliOptions;
};

template<class... Params>
auto make_program_options(Params... params)
{
  auto f = [&]{
    return [params...](auto cli_f){
      return cli_f(params...);
    };
  };
  return ProgramOptions<decltype(f())>{f()};
}

#define CLI_ERR_IF(x, msg) do { if (x) throw runtime_cli_error{msg}; } while (0)

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

template<class T> T ston(const char* s, char** end) = delete;

template<> inline int ston<int>(const char* s, char** end)
{ return strtoi(s, end, 10); }

template<> inline long ston<long>(const char* s, char** end)
{ return strtol(s, end, 10); }

template<> inline long long ston<long long>(const char* s, char** end)
{ return strtoll(s, end, 10); }

template<> inline unsigned ston<unsigned>(const char* s, char** end)
{ return strtou(s, end, 10); }

template<> inline unsigned long ston<unsigned long>(const char* s, char** end)
{ return strtoul(s, end, 10); }

template<> inline unsigned long long ston<unsigned long long>(const char* s, char** end)
{ return strtoull(s, end, 10); }

template<> inline double ston<double>(const char* s, char** end)
{ return strtod(s, end); }

template<> inline long double ston<long double>(const char* s, char** end)
{ return strtold(s, end); }

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

namespace detail
{
  namespace {
    enum class State {
      unknown, fail, ok, ok_arg
    };
  }
}

template<class CliOptions>
template<class... Args>
int ProgramOptions<CliOptions>::parse_command_line(
  int ac, char const* const* av, Args&&... args)
{
  using State = detail::State;

  auto for_each_options = [this](auto f){
    return mCliOptions([&f,this](auto & ... opts){
      State st = State::unknown;
      FALCON_UNPACK(
        st == State::unknown
        ? void(st = f(opts))
        : void()
      );
      CLI_ERR_IF(st == State::unknown, "unknown option");
      return st;
    });
  };

  int optint = 0;
  char current_single_opt[2] {};
  zstring current_long_opt = nullptr;

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
        for_each_options([&](auto const & opt){
          // TODO = support: --name=value and --name=
          if (opt.long_name() == zview) {
            switch (opt.has_argument()) {
              case HasArgument::No:
                opt.call(nullptr, args...);
                return State::ok;
              case HasArgument::Required:
                CLI_ERR_IF(
                  optint+1 == ac,
                  std::string("`--") + zview.c_str() + "` requires value"
                );
                opt.call(av[optint+1], args...);
                ++optint;
                return State::ok;
              case HasArgument::Optional:
                // TODO unimplemented
                assert(false && "unimplemented");
                CLI_ERR_IF(true, "unimplemented");
                break;
            }
          }
          return State::unknown;
        });
      }
      // short option
      else {
        current_long_opt = nullptr;
        current_single_opt[0] = 0;
        while (*++arg) {
          current_single_opt[0] = *arg;
          // TODO use falcon.cstmt::cswitch
          auto st = for_each_options([&](auto const & opt){
            if (opt.short_name() && opt.short_name() == *arg) {
              switch (opt.has_argument()) {
                case HasArgument::No:
                  opt.call(nullptr, args...);
                  return State::ok;
                case HasArgument::Required:
                  if (arg[1]) {
                    opt.call(arg+1, args...);
                    --optint;
                    return State::ok_arg;
                  }
                  CLI_ERR_IF(
                    optint+1 == ac,
                    std::string("-`") + opt.short_name() + "` requires value"
                  );
                  opt.call(av[optint+1], args...);
                  return State::ok_arg;
                case HasArgument::Optional:
                  // TODO unimplemented
                  assert(false && "unimplemented");
                  CLI_ERR_IF(true, "unimplemented");
              }
            }
            return State::unknown;
          });
          if (State::ok_arg == st) {
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

template<class CliOptions>
void ProgramOptions<CliOptions>::help()
{
  return mCliOptions([](auto & ... opts){
    FALCON_UNPACK(
      opts.short_name() && opts.long_name()
      ? void(std::cout
        << '-' << opts.short_name()
        << ", --" << opts.long_name()
        << "  " << opts.help() << '\n'
      )
      : opts.short_name()
      ? void(std::cout
        << '-' << opts.short_name()
        << "  " << opts.help() << '\n'
      )
      : void(std::cout
        << "--" << opts.long_name()
        << "  " << opts.help() << '\n'
      )
    );
  });
}

}
}
