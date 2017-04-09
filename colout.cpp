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

#include "colout/cli/parse_cli.hpp"
#include "colout/utils/limited_array.hpp"
#include "colout/utils/range.hpp"

#include <falcon/cxx/cxx.hpp>

#include <type_traits>

#include <iostream>
#include <iterator>

#ifndef IN_IDE_PARSER
template<class... T>
#endif
void TRACE(T const&... args)
{
  FALCON_UNPACK(std::cerr << args);
  std::cerr << "\n";
}


using std::begin;
using std::end;

namespace colout
{
  constexpr string_view esc_reset
    FALCON_IN_IDE_PARSER_CONDITIONAL(FALCON_PP_NIL, = "\033[0m"_sv);


  // Variant
  //@{
  template<class T>
  struct PlaceType
  {
    explicit PlaceType() = default;
    using type = T;
  };

  namespace detail
  {
    template<class Ints, class... Ts>
    struct IndexedType;

    template<class i, class T>
    struct PairType {};

    template<std::size_t... I, class... Ts>
    struct IndexedType<std::integer_sequence<std::size_t, I...>, Ts...>
    : PairType<std::integral_constant<std::size_t, I>, Ts>...
    {};

    template<class T, class I>
    I get_index_of(PairType<I, T>*);
  }

  template<class T, class... Ts>
  using indexOf = decltype(detail::get_index_of<T>(
    static_cast<
      detail::IndexedType<
        std::index_sequence_for<Ts...>,
        Ts...
      >*
    >(nullptr)
  ));

  template<class R, class F, class T, class... Args>
  struct PtrFuncHelper
  {
    static R impl(F&& f, void * d, Args&&... args)
    {
      return f(*static_cast<T*>(d), std::forward<Args>(args)...);
    }
  };

  template<class T, class... Ts>
  struct Variant
  {
    template<class U, class... Args>
    Variant(PlaceType<U>, Args && ... args)
    : idx_(indexOf<U, T, Ts...>::value)
    {
      new (&data_) U {std::forward<Args>(args)...};
    }

    Variant(Variant && other)
    : idx_(other.idx_)
    {
      other.visit([this, &other](auto & d) {
        using U = std::remove_reference_t<decltype(d)>;
        new (&this->data_) U {std::move(*static_cast<T*>(other.d))};
      });
    }

    Variant(Variant const &) = delete;

    Variant operator=(Variant const &) = delete;

    ~Variant()
    {
      visit([](auto & d) {
        using U = std::remove_reference_t<decltype(d)>;
        d.~U();
      });
    }

    template<
      class F, class... Args,
      class R = decltype(
        std::declval<F&&>()(std::declval<T&>(), std::declval<Args&&>()...)
      )
    >
    R visit(F && f, Args && ... args)
    {
      using proto = R(*)(F &&, void *, Args...);
      static constexpr proto func_ptrs[]{
        PtrFuncHelper<R, F, T, Args...>::impl,
        PtrFuncHelper<R, F, Ts, Args...>::impl...
      };
      return func_ptrs[idx_](std::forward<F>(f), &data_, args...);
    }

    typename std::aligned_union_t<0, T, Ts...>::type data_;
    int idx_;
  };
  //@}


  struct VisitorResult
  {
    bool isFound;
    int nextId;
    std::size_t countConsumed;
  };

  struct BranchCtx
  {
    int nextIdOk;
    int nextIdFail;

    int computeNextId(bool ok) const noexcept
    {
      return ok ? nextIdOk : nextIdFail;
    }
  };

  class Scanner;

  VisitorResult run_at(
    Scanner& scanner, int id, std::string& ctx, string_view sv
  );

  struct ColorApplicator
  {
    ColorApplicator(Color color)
    : mColorOrId(PlaceType<std::string>{}, color.str_move())
    {}

    ColorApplicator(int id)
    : mColorOrId(PlaceType<int>{}, id)
    {}

    bool apply(Scanner& scanner, std::string& ctx, string_view sv)
    {
      struct Fns
      {
        bool operator()(
          std::string const& s,
          Scanner&, std::string& ctx, string_view sv)
        {
          ctx
            .append(begin(s), end(s))
            .append(begin(sv), end(sv))
          ;
          return true;
        }

        bool operator()(
          int id,
          Scanner& scanner, std::string& ctx, string_view sv)
        {
          std::size_t pos = 0;

          auto res = run_at(scanner, id, ctx, sv);
          while (res.nextId != -1)
          {
            pos += res.countConsumed;
            res = run_at(scanner, res.nextId, ctx, sv.substr(pos));
          }
          return res.isFound;
        }
      };
      return mColorOrId.visit(Fns{}, scanner, ctx, sv);
    }

  private:
    Variant<std::string, int> mColorOrId;
  };

  struct ColorSelector
  {
    ColorSelector(std::size_t count, bool loop)
    : mCount(int(count))
    , mMaxColor(mCount-1)
    , mModColor(loop ? mCount : std::numeric_limits<int>::max())
    {
    }

    std::size_t next_idx()
    {
      auto i = std::min(mIColor % mModColor, mMaxColor);
      ++mIColor;
      return std::size_t(i);
    }

    void reset()
    {
      mIColor = 0;
    }

  private:
    int mCount;
    int mMaxColor;
    int mModColor;
    int mIColor = 0;
  };

  using F = cli::ActiveFlags;

  struct Pattern
  {
    Pattern(
      BranchCtx bctx,
      std::regex reg,
      limited_array<ColorApplicator> colors,
      std::string esc_reset,
      F f
    )
    : mReg(std::move(reg))
    , mColorSelector(colors.size(), bool(F::loop_color & f))
    , mColors(std::move(colors))
    , mEscReset(std::move(esc_reset))
    , mResetColor(!bool(F::keep_color & f))
    , mContinueFromLastColor(bool(F::continue_from_last_color & f))
    , mEndColorMark(bool(F::end_color_mark & f))
    , mBCtx(bctx)
    {}

    VisitorResult run(Scanner& scanner, std::string& ctx, string_view sv)
    {
      std::size_t pos = 0;
      std::size_t const ctx_sz_saved = ctx.size();
      bool const exists = std::regex_search(sv.begin(), sv.end(), mMatch, mReg);

      if (exists)
      {
        auto apply_color = [&](size_t isub){
          auto const new_pos = mMatch.position(isub);
          auto const len = mMatch.length(isub);
          ctx.append(begin(sv) + pos,      begin(sv) + new_pos);
          if (!mColors[mColorSelector.next_idx()]
            .apply(scanner, ctx, sv.substr(new_pos, len)))
          {
            ctx.resize(ctx_sz_saved);
            return false;
          }
          ctx.append(begin(mEscReset),     end(mEscReset));
          pos = std::size_t(new_pos + len);
          return true;
        };

        if (!mReg.mark_count())
        {
          if (!apply_color(0)) {
            return {false, mBCtx.nextIdFail, 0};
          }
        }
        else
        {
          for (size_t i : range(1u, mMatch.size()))
          {
            if (!apply_color(i)) {
              return {false, mBCtx.nextIdFail, 0};
            }
          }

          if (!mContinueFromLastColor)
          {
            auto const newPos = mMatch.length(0);
            ctx.append(begin(sv) + pos, begin(sv) + newPos);
            pos = newPos;
          }
        }

        if (mEndColorMark)
        {
          if (!mColors[mColorSelector.next_idx()].apply(scanner, ctx, sv))
          {
            ctx.resize(ctx_sz_saved);
            return {false, mBCtx.nextIdFail, 0};
          }
        }

        if (mResetColor)
        {
          mColorSelector.reset();
        }
      }

      return VisitorResult{exists, mBCtx.computeNextId(exists), pos};
    }

  private:
    std::regex mReg;
    std::cmatch mMatch;
    ColorSelector mColorSelector;
    limited_array<ColorApplicator> mColors;
    std::string mEscReset;
    bool mResetColor;
    bool mContinueFromLastColor;
    bool mEndColorMark;
    BranchCtx mBCtx;
  };

  template<class Pattern>
  struct Loop
  {
    template<class... Args>
    Loop(Args&&... args)
    : mPattern(std::forward<Args>(args)...)
    {}

    VisitorResult run(Scanner& scanner, std::string& ctx, string_view sv)
    {
      std::size_t pos = 0;
      auto res = mPattern.run(scanner, ctx, sv);
      if (res.isFound)
      {
        int const nextId = res.nextId;
        do {
          pos += res.countConsumed;
          res = mPattern.run(scanner, ctx, sv.substr(pos));
        } while (res.isFound);
        res.isFound = true;
        res.countConsumed = pos;
        res.nextId = nextId;
      }
      return res;
    }

  private:
    Pattern mPattern;
  };

  template<class Pattern>
  struct Group
  {
    template<class... Args>
    Group(BranchCtx bctx, Args&&... args)
    : mPattern(std::forward<Args&&>(args)...)
    , mBCtx(bctx)
    {}

    VisitorResult run(Scanner& scanner, std::string& ctx, string_view sv)
    {
      std::size_t pos = 0;
      std::size_t const ctx_sz_saved = ctx.size();

      auto res = mPattern.run(scanner, ctx, sv);
      while (res.nextId != -1)
      {
        pos += res.countConsumed;
        res = run_at(scanner, res.nextId, ctx, sv.substr(pos));
      }

      if (res.isFound)
      {
        pos += res.countConsumed;
        return {true, mBCtx.nextIdOk, pos};
      }
      else
      {
        ctx.resize(ctx_sz_saved);
        return {false, mBCtx.nextIdFail, 0};
      }
    }

  private:
    Pattern mPattern;
    BranchCtx mBCtx;
  };

  struct Jump
  {
    Jump(int i)
    : mI(i)
    {}

    VisitorResult run(Scanner& scanner, std::string& ctx, string_view sv)
    {
      return run_at(scanner, mI, ctx, sv);
    }

  private:
    int mI;
  };

  struct Call
  {
    Call(BranchCtx bctx, int i)
    : mI(i)
    , mBCtx(bctx)
    {}

    VisitorResult run(Scanner& scanner, std::string& ctx, string_view sv)
    {
      auto res = run_at(scanner, mI, ctx, sv);
      res.nextId = mBCtx.computeNextId(res.isFound);
      return res;
    }

  private:
    int mI;
    BranchCtx mBCtx;
  };

  struct TestPattern
  {
    TestPattern(int i, std::regex reg)
    : mReg(std::move(reg))
    , mI(i)
    {}

    VisitorResult run(Scanner&, std::string&, string_view sv)
    {
      bool const exists = std::regex_search(sv.begin(), sv.end(), mReg);
      return {exists, exists ? mI : -1, 0};
    }

  private:
    std::regex mReg;
    int mI;
  };

  struct TestPatternAndCall
  {
    TestPatternAndCall(BranchCtx bctx, int i, std::regex reg)
    : mReg(std::move(reg))
    , mI(i)
    , mBCtx(bctx)
    {}

    VisitorResult run(Scanner& scanner, std::string& ctx, string_view sv)
    {
      bool exists = std::regex_search(sv.begin(), sv.end(), mReg);
      if (exists)
      {
        auto res = run_at(scanner, mI, ctx, sv);
        res.nextId = mBCtx.computeNextId(res.isFound);
      }
      return {false, mBCtx.nextIdFail, 0};
    }

  private:
    std::regex mReg;
    int mI;
    BranchCtx mBCtx;
  };


  struct Scanner
  {
    template<class... Ts>
    using Variant_ = Variant<Ts..., Loop<Ts>...>;

    struct Element : Variant_<
      Pattern,
      TestPattern,
      TestPatternAndCall,
      Group<TestPattern>,
      Group<Jump>,
      Jump,
      Call
    >
    {
      FALCON_DIAGNOSTIC_PUSH
      FALCON_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wuseless-cast")
      using Variant::Variant;
      FALCON_DIAGNOSTIC_POP
    };

    limited_array<Element> elems;
  };

  VisitorResult run_at(Scanner& scanner, int id, std::string& ctx, string_view sv)
  {
    assert(id != -1);
    TRACE("run_at: ", id, " ", sv);
    return scanner.elems[id].visit([&](auto & p){
      return p.run(scanner, ctx, sv);
    });
  }


  using ColoutParamCRef = cli::ColoutParam const&;

  inline int next_id(int id, ColoutParamCRef param)
  {
    if (param.bind_index != -1)
    {
      id = param.bind_index;
    }
    return id + 1;
  }

  inline int get_next_ok(std::vector<cli::ColoutParam> const & params, int i)
  {
    ColoutParamCRef param = params[i];

    if (bool(F::next_is_sub & param.activated_flags))
    {
      return i + 1;
    }

    while (i < int(params.size()))
    {
      ColoutParamCRef curr = params[i];
      if (bool(F::next_is_seq & curr.activated_flags))
      {
        return next_id(i, curr);
      }
      else if (bool(F::next_is_alt & curr.activated_flags))
      {
        i = next_id(i, curr);
      }
      else
      {
        return -1;
      }
    }

    return -1;
  }

  inline int get_next_fail(std::vector<cli::ColoutParam> const & params, int i)
  {
    ColoutParamCRef param = params[i];

    if (bool(F::next_is_alt & param.activated_flags))
    {
      return next_id(i, param);
    }
    return -1;
  }

  template<class T>
  using t_ = typename T::type;

  inline Scanner make_scanner(std::vector<cli::ColoutParam> params)
  {
    limited_array<Scanner::Element> elems(params.size());

    for (int const i : range(0, int(params.size())))
    {
      BranchCtx bctx{
        get_next_ok(params, i),
        get_next_fail(params, i)
      };
      ColoutParamCRef param = params[i];

      std::cerr << i << " -> " << bctx.nextIdOk << "  " << bctx.nextIdFail << "\n";

      auto mk_maybe_loop = [&](auto t, auto mk){
        if (bool(F::loop_regex & param.activated_flags))
        {
          mk(PlaceType<Loop<t_<decltype(t)>>>{});
        }
        else
        {
          mk(t);
        }
      };

      if (bool(F::regex & param.activated_flags))
      {
        std::regex reg(param.regex.c_str());

        if (bool(F::next_is_sub & param.activated_flags))
        {
          mk_maybe_loop(PlaceType<Group<TestPattern>>{}, [&](auto t){
            elems.emplace_back(t, bctx, int(elems.size() + 1u), std::move(reg));
          });
        }
        else if (param.go_id != -1)
        {
          if (bool(F::call_label | param.activated_flags))
          {
            mk_maybe_loop(PlaceType<TestPatternAndCall>{}, [&](auto t){
              elems.emplace_back(t, bctx, param.go_id, std::move(reg));
            });
          }
          else
          {
            mk_maybe_loop(PlaceType<TestPattern>{}, [&](auto t){
              elems.emplace_back(t, param.go_id, std::move(reg));
            });
          }
        }
        else
        {
          limited_array<ColorApplicator> colors(param.colors.size());
          TRACE("color.sz: ", param.colors.size());
          assert(param.colors.size());
          for (cli::ColorParam const & color :  param.colors)
          {
            if (color.id_label != -1)
            {
              TRACE("color.label: ", color.id_label);
              colors.emplace_back(color.id_label);
            }
            else
            {
              TRACE("color.color: ", color.color);
              colors.emplace_back(color.color);
            }
          }

          mk_maybe_loop(PlaceType<Pattern>{}, [&](auto t){
            elems.emplace_back(
              t,
              bctx,
              std::move(reg),
              std::move(colors),
              bool(F::set_reset_color & param.activated_flags)
                ? std::move(param.esc)
                : esc_reset.data(),
              param.activated_flags
            );
          });
        }
      }
      else if (bool(F::start_group & param.activated_flags))
      {
        mk_maybe_loop(PlaceType<Group<Jump>>{}, [&](auto t){
          elems.emplace_back(t, bctx, int(elems.size() + 1u));
        });
      }
      else if (param.go_id != -1)
      {
        if (bool(F::call_label | param.activated_flags))
        {
          mk_maybe_loop(PlaceType<Call>{}, [&](auto t){
            elems.emplace_back(t, bctx, param.go_id);
          });
        }
        else
        {
          mk_maybe_loop(PlaceType<Jump>{}, [&](auto t){
            elems.emplace_back(t, param.go_id);
          });
        }
      }
    }

    return Scanner{std::move(elems)};
  }
}

namespace cli = colout::cli;
using F = cli::ActiveFlags;


int main(int ac, char ** av)
{
  std::vector<cli::ColoutParam> colout_params = cli::colout_parse_cli(ac, av);

  unsigned i = 0;
  for (cli::ColoutParam const & colout_param : colout_params)
  {
    std::cerr << i++ << "\n"
      << "  r(" << bool(colout_param.activated_flags & F::regex) << "): "
      << colout_param.regex << "\n"
      << "  loop: " << bool(colout_param.activated_flags & F::loop_flags)
      << "  sub: " << bool(colout_param.activated_flags & F::next_is_sub)
      << "  or: " << bool(colout_param.activated_flags & F::next_is_alt)
      << "  seq: " << bool(colout_param.activated_flags & F::next_is_seq)
      << "  label: " << colout_param.label
      << "\n"
    ;
  }

  colout::Scanner scanner = colout::make_scanner(std::move(colout_params));

  std::string s;
  std::string ctx;
  while (std::getline(std::cin, s))
  {
    int i = 0;
    colout::string_view sv{s.data(), s.size()};
    do
    {
      auto ret = colout::run_at(scanner, i, ctx, sv);
      sv.remove_prefix(ret.countConsumed);
      i = ret.nextId;
    } while (i != -1);

    std::cout << ctx;
    std::cout.write(sv.data(), sv.size()) << '\n';
    ctx.clear();
  }
}
