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

namespace detail
{
  namespace {
    enum class State {
      unknown, fail, ok, ok_arg
    };
  }
}

#define CLI_ERR_IF(x, msg) do { if (x) throw runtime_cli_error{msg}; } while (0)

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
