/* The MIT License (MIT)

Copyright (c) 2017 jonathan poelen

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
#include "colout/utils/overload.hpp"

#include <falcon/cxx/cxx.hpp>

#include <type_traits>

#include <iostream>
#include <iterator>
#include <random>
#include <memory>

#include <mpark/variant.hpp>

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

  template<class Variant, class F, class... Args>
  auto visit(Variant & v, F f, Args&&... args)
  //-> decltype(f(mpark::get<0>(v), std::forward<Args>(args)...))
  {
    // hack: bypassing value check
    return mpark::detail::visitation::variant::visit_value([&](auto & e){
      return f(e, std::forward<Args>(args)...);
    }, v);
  }

  using mpark::in_place_type_t;
  using mpark::in_place_type;

  enum class ColoutIndex : int {};
  constexpr ColoutIndex invalidIndex = ColoutIndex(-1);
  constexpr ColoutIndex zeroAsIndex = ColoutIndex(0);

  template<class Ch, class Tr>
  std::basic_ostream<Ch, Tr>&
  operator<<(std::basic_ostream<Ch, Tr>& out, ColoutIndex const & i)
  {
    return out << static_cast<std::underlying_type_t<ColoutIndex>>(i);
  }

  struct VisitorResult
  {
    bool isFound;
    ColoutIndex nextId;
    std::size_t countConsumed;
  };

  struct BranchCtx
  {
    ColoutIndex nextIdOk;
    ColoutIndex nextIdFail;

    ColoutIndex computeNextId(bool ok) const noexcept
    {
      return ok ? nextIdOk : nextIdFail;
    }
  };

  class Scanner;

  VisitorResult run_at(
    Scanner& scanner, ColoutIndex id, std::string& ctx, string_view sv,
    std::size_t currentLoopIdx
  );

  struct VisitorBase
  {
    virtual ~VisitorBase() {}
    virtual VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentLoopIdx) /* TODO const */ = 0;
  };

  struct ColorApplicatorBase
  {
    virtual ~ColorApplicatorBase() {}
    virtual bool apply(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentColorIdx) = 0;
  };

  struct IndexColorApplicator : ColorApplicatorBase
  {
    IndexColorApplicator(ColoutIndex id)
    : mId(id)
    {}

    bool apply(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentColorIdx) override
    {
      std::size_t pos = 0;

      auto res = run_at(scanner, mId, ctx, sv, currentColorIdx);
      while (res.nextId != invalidIndex)
      {
        ++currentColorIdx;
        pos += res.countConsumed;
        res = run_at(scanner, res.nextId, ctx, sv.substr(pos), currentColorIdx);
      }
      return res.isFound;
    }

  private:
    ColoutIndex mId;
  };

  struct SingleColorApplicator : ColorApplicatorBase
  {
    SingleColorApplicator(Color color)
    : mColor(std::move(color))
    {}

    bool apply(
      Scanner&, std::string& ctx, string_view sv,
      std::size_t /*currentColorIdx*/) override
    {
      ctx
        .append(begin(mColor.str()), end(mColor.str()))
        .append(begin(sv), end(sv))
      ;
      return true;
    }

  private:
    Color mColor;
  };

  using hash_type = uint32_t;
  inline hash_type hash(string_view sv)
  {
    // DJB2 hash variant
    hash_type h = 0;
    for (char c : sv) {
      h ^= (h << 5) + (h >> 2) + c;
    }
    return h;
  }

  struct HashColorApplicator : ColorApplicatorBase
  {
    HashColorApplicator(std::vector<Color> colors)
    : mColors(std::move(colors))
    {}

    bool apply(
      Scanner&, std::string& ctx, string_view sv,
      std::size_t /*currentColorIdx*/) override
    {
      auto const & color = mColors[hash(sv) % mColors.size()];
      ctx
        .append(begin(color.str()), end(color.str()))
        .append(begin(sv), end(sv))
      ;
      return true;
    }

  private:
    std::vector<Color> mColors;
  };

  struct SimpleCycleColorApplicator : ColorApplicatorBase
  {
    SimpleCycleColorApplicator(std::vector<Color> colors)
    : mColors(std::move(colors))
    {}

    bool apply(
      Scanner&, std::string& ctx, string_view sv,
      std::size_t currentColorIdx) override
    {
      auto const & color = mColors[std::min(currentColorIdx, mColors.size()-1u)];
      ctx
        .append(begin(color.str()), end(color.str()))
        .append(begin(sv), end(sv))
      ;
      return true;
    }

  private:
    std::vector<Color> mColors;
  };

  struct RandomColorApplicator : ColorApplicatorBase
  {
    RandomColorApplicator(std::vector<Color> colors, unsigned seed = 0)
    : mColors(std::move(colors))
    , mEngine(seed)
    , mDist(0, mColors.size()-1u)
    {}

    bool apply(
      Scanner&, std::string& ctx, string_view sv,
      std::size_t /*currentColorIdx*/) override
    {
      auto const & color = mColors[mDist(mEngine)];
      ctx
        .append(begin(color.str()), end(color.str()))
        .append(begin(sv), end(sv))
      ;
      return true;
    }

  private:
    std::vector<Color> mColors;
    std::default_random_engine mEngine;
    std::uniform_int_distribution<std::size_t> mDist;
  };

  struct SimpleScaleColorApplicator : ColorApplicatorBase
  {
    SimpleScaleColorApplicator(std::vector<Color> colors, double min, double max)
    : mColors(std::move(colors))
    , mMin(min)
    , mMax(max)
    {
      assert(min < max);
    }

    bool apply(
      Scanner&, std::string& ctx, string_view sv,
      std::size_t /*currentColorIdx*/) override
    {
      char & mutableChar = const_cast<char&>(sv.data()[sv.size()]);
      auto const oldChar = std::exchange(mutableChar, '\0');
      double val = strtod(sv.data(), nullptr);
      mutableChar = oldChar;

      auto const & color = mColors[
        (errno == ERANGE)
        ? 0
        : std::min(
          std::size_t((std::max(mMin, std::min(val, mMax)) - mMin) / mDiv),
          mMaxIdx
        )
      ];
      ctx
        .append(begin(color.str()), end(color.str()))
        .append(begin(sv), end(sv))
      ;
      return true;
    }

  private:
    std::vector<Color> mColors;
    double mMin;
    double mMax;
    double mDiv = std::abs(mMax-mMin) / double(mColors.size());
    std::size_t mMaxIdx = mColors.size() - 1;
  };

  struct IndexComputer
  {
    IndexComputer(std::size_t count, bool loop)
   : mCount(count)
   , mMaxColor(mCount-1)
   , mModColor(loop ? mCount : std::numeric_limits<int>::max())
    {
    }

    std::size_t computeIdx(std::size_t i) const
    {
      return std::min(i % mModColor, mMaxColor);
    }

  private:
    std::size_t mCount;
    std::size_t mMaxColor;
    std::size_t mModColor;
  };

  struct CycleColorApplicator : ColorApplicatorBase
  {
    CycleColorApplicator(std::vector<Color> colors, bool loop)
    : mColors(std::move(colors))
    , mIndexComputer(mColors.size(), loop)
    {}

    bool apply(
      Scanner&, std::string& ctx, string_view sv,
      std::size_t currentColorIdx) override
    {
      auto const & color = mColors[mIndexComputer.computeIdx(currentColorIdx)];
      ctx
        .append(begin(color.str()), end(color.str()))
        .append(begin(sv), end(sv))
      ;
      return true;
    }

  private:
    std::vector<Color> mColors;
    IndexComputer mIndexComputer;
  };

  struct ColorApplicator
  {
    template<class T, class... Args>
    ColorApplicator(mpark::in_place_type_t<T>, Args&&... args)
    : p(new T{std::forward<Args>(args)...})
    {
    }

    bool apply(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentColorIdx)
    {
      return p->apply(scanner, ctx, sv, currentColorIdx);
    }

  private:
    std::unique_ptr<ColorApplicatorBase> p;
  };

  using F = cli::ActiveFlags;

  struct Pattern : VisitorBase
  {
    Pattern(
      BranchCtx bctx,
      std::regex reg,
      limited_array<ColorApplicator> colors,
      std::string esc_reset,
      F f
    )
    : mReg(std::move(reg))
    , mIndexComputer(colors.size(), bool(F::loop_color & f))
    , mColors(std::move(colors))
    , mEscReset(std::move(esc_reset))
    , mResetColor(!bool(F::keep_color & f))
    , mContinueFromLastColor(bool(F::continue_from_last_color & f))
    , mEndColorMark(bool(F::end_color_mark & f))
    , mBCtx(bctx)
    {}

    VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentLoopIdx) override
    {
      std::size_t pos = 0;
      std::size_t const ctx_sz_saved = ctx.size();
      bool const exists = std::regex_search(sv.begin(), sv.end(), mMatch, mReg);

      if (exists)
      {
        std::size_t currentColorIdx = mResetColor ? 0u : currentLoopIdx;

        auto apply_color = [&](size_t isub){
          auto const new_pos = mMatch.position(isub);
          auto const len = mMatch.length(isub);
          ctx.append(begin(sv) + pos, begin(sv) + new_pos);
          if (!mColors[mIndexComputer.computeIdx(currentColorIdx)]
            .apply(scanner, ctx, sv.substr(new_pos, len), currentLoopIdx))
          {
            ctx.resize(ctx_sz_saved);
            return false;
          }
          ++currentColorIdx;
          ++currentLoopIdx;
          ctx.append(begin(mEscReset), end(mEscReset));
          pos = std::size_t(new_pos + len);
          return true;
        };

        if (!mReg.mark_count())
        {
          if (!apply_color(0))
          {
            return {false, mBCtx.nextIdFail, 0};
          }
        }
        else
        {
          for (size_t i : range(1u, mMatch.size()))
          {
            if (!apply_color(i))
            {
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
          if (!mColors[mIndexComputer.computeIdx(currentColorIdx)]
            .apply(scanner, ctx, sv, currentLoopIdx))
          {
            ctx.resize(ctx_sz_saved);
            return {false, mBCtx.nextIdFail, 0};
          }
        }
      }

      return VisitorResult{exists, mBCtx.computeNextId(exists), pos};
    }

  private:
    std::regex mReg;
    std::cmatch mMatch;
    IndexComputer mIndexComputer;
    limited_array<ColorApplicator> mColors;
    std::string mEscReset;
    bool mResetColor;
    bool mContinueFromLastColor;
    bool mEndColorMark;
    BranchCtx mBCtx;
  };

  template<class Impl>
  struct Loop : VisitorBase
  {
    template<class... Args>
    Loop(Args&&... args)
    : mImpl(std::forward<Args>(args)...)
    {}

    VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t /*currentLoopIdx*/) override
    {
      std::size_t pos = 0;
      std::size_t currentLoopIdx = 0;
      auto res = mImpl.run(scanner, ctx, sv, currentLoopIdx);
      if (res.isFound)
      {
        ColoutIndex const nextId = res.nextId;
        do
        {
          ++currentLoopIdx;
          pos += res.countConsumed;
          res = mImpl.run(scanner, ctx, sv.substr(pos), currentLoopIdx);
        } while (res.isFound);
        res.isFound = true;
        res.countConsumed = pos;
        res.nextId = nextId;
      }
      return res;
    }

  private:
    Impl mImpl;
  };

  template<class Impl>
  struct Group : VisitorBase
  {
    template<class... Args>
    Group(BranchCtx bctx, Args&&... args)
    : mImpl(std::forward<Args&&>(args)...)
    , mBCtx(bctx)
    {}

    VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentLoopIdx) override
    {
      std::size_t pos = 0;
      std::size_t const ctx_sz_saved = ctx.size();

      auto res = mImpl.run(scanner, ctx, sv, currentLoopIdx);
      while (res.nextId != invalidIndex)
      {
        pos += res.countConsumed;
        res = run_at(scanner, res.nextId, ctx, sv.substr(pos), currentLoopIdx);
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
    Impl mImpl;
    BranchCtx mBCtx;
  };

  struct Jump : VisitorBase
  {
    Jump(ColoutIndex i)
    : mI(i)
    {}

    VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentLoopIdx) override
    {
      return run_at(scanner, mI, ctx, sv, currentLoopIdx);
    }

  private:
    ColoutIndex mI;
  };

  struct Call : VisitorBase
  {
    Call(BranchCtx bctx, ColoutIndex i)
    : mI(i)
    , mBCtx(bctx)
    {}

    VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentLoopIdx) override
    {
      auto res = run_at(scanner, mI, ctx, sv, currentLoopIdx);
      res.nextId = mBCtx.computeNextId(res.isFound);
      return res;
    }

  private:
    ColoutIndex mI;
    BranchCtx mBCtx;
  };

  struct TestPattern : VisitorBase
  {
    TestPattern(ColoutIndex i, std::regex reg)
    : mReg(std::move(reg))
    , mI(i)
    {}

    VisitorResult run(
      Scanner&, std::string&, string_view sv,
      std::size_t /*currentLoopIdx*/) override
    {
      bool const exists = std::regex_search(sv.begin(), sv.end(), mReg);
      return {exists, exists ? mI : invalidIndex, 0};
    }

  private:
    std::regex mReg;
    ColoutIndex mI;
  };

  struct TestPatternAndCall : VisitorBase
  {
    TestPatternAndCall(BranchCtx bctx, ColoutIndex i, std::regex reg)
    : mReg(std::move(reg))
    , mI(i)
    , mBCtx(bctx)
    {}

    VisitorResult run(
      Scanner& scanner, std::string& ctx, string_view sv,
      std::size_t currentLoopIdx) override
    {
      bool exists = std::regex_search(sv.begin(), sv.end(), mReg);
      if (exists)
      {
        auto res = run_at(scanner, mI, ctx, sv, currentLoopIdx);
        res.nextId = mBCtx.computeNextId(res.isFound);
      }
      return {false, mBCtx.nextIdFail, 0};
    }

  private:
    std::regex mReg;
    ColoutIndex mI;
    BranchCtx mBCtx;
  };


  struct Scanner
  {
    struct Element
    {
      template<class T, class... Args>
      Element(mpark::in_place_type_t<T>, Args&&... args)
      : p(new T{std::forward<Args>(args)...})
      {
      }

      VisitorResult run(
        Scanner& scanner, std::string& ctx, string_view sv,
        std::size_t currentLoopIdx)
      {
        return p->run(scanner, ctx, sv, currentLoopIdx);
      }

    private:
      std::unique_ptr<VisitorBase> p;
    };

    std::vector<Element> elems;
  };

  VisitorResult run_at(
    Scanner& scanner, ColoutIndex id, std::string& ctx, string_view sv,
    std::size_t currentLoopIdx)
  {
    assert(id != invalidIndex);
    TRACE("run_at: ", id, " ", sv);
    return scanner.elems[static_cast<std::size_t>(id)]
      .run(scanner, ctx, sv, currentLoopIdx);
  }

  using ColoutParamCRef = cli::ColoutParam const&;
  using ColoutParamRef = cli::ColoutParam&;

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

  template<class>
  struct extract_in_place_type;

  template<class T>
  struct extract_in_place_type<mpark::in_place_type_t<T>>
  { using type = T; };

  template<class T>
  using extract_in_place_type_t = typename extract_in_place_type<T>::type;

  inline Scanner make_scanner(std::vector<cli::ColoutParam> params)
  {
    decltype(Scanner::elems) elems;
    elems.reserve(params.size());

    for (int const i : range(0, int(params.size())))
    {
      BranchCtx bctx{
        ColoutIndex(get_next_ok(params, i)),
        ColoutIndex(get_next_fail(params, i))
      };
      ColoutParamRef param = params[i];

      TRACE(i, " -> ", bctx.nextIdOk, "  ", bctx.nextIdFail     );

      auto mk_maybe_loop = [&](auto t, auto mk){
        if (bool(F::loop_regex & param.activated_flags))
        {
          mk(in_place_type_t<Loop<extract_in_place_type_t<decltype(t)>>>{});
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
          mk_maybe_loop(in_place_type_t<Group<TestPattern>>{}, [&](auto t){
            elems.emplace_back(
              t, bctx,
              ColoutIndex(elems.size() + 1u),
              std::move(reg)
            );
          });
        }
        else if (param.go_id != -1)
        {
          if (bool(F::call_label | param.activated_flags))
          {
            mk_maybe_loop(in_place_type_t<TestPatternAndCall>{}, [&](auto t){
              elems.emplace_back(
                t, bctx,
                ColoutIndex(param.go_id),
                std::move(reg)
              );
            });
          }
          else
          {
            mk_maybe_loop(in_place_type_t<TestPattern>{}, [&](auto t){
              elems.emplace_back(
                t,
                ColoutIndex(param.go_id),
                std::move(reg)
              );
            });
          }
        }
        else
        {
          limited_array<ColorApplicator> color_applicators(param.colors.size());
          TRACE("color_param.sz: ", param.colors.size());
          assert(param.colors.size());
          for (cli::ColorParam& color_param : param.colors)
          {
            visit(color_param.color, overload(
              [](cli::ColorParam::ThemeName){ assert(false); },
              [](cli::ColorParam::LabelName){ assert(false); },
              [&](Color& color){
                TRACE("color_param.color: ", color);
                color_applicators.emplace_back(
                  in_place_type_t<SingleColorApplicator>{},
                  std::move(color)
                );
              },
              [&](cli::ColorParam::LabelId id){
                TRACE("color_param.label: ", int(id));
                color_applicators.emplace_back(
                  in_place_type_t<IndexColorApplicator>{},
                  ColoutIndex(int(id))
                );
              },
              [&](cli::ColorParam::Colors& param){
                auto& colors = param.colors;
                assert(!colors.empty());
                TRACE("color_param.colors: count=", colors.size());
                if (colors.size() == 1)
                {
                  color_applicators.emplace_back(
                    in_place_type_t<SingleColorApplicator>{},
                    std::move(colors[0])
                  );
                }
                else
                {
                  visit(param.mode, overload(
                    [&](mpark::monostate){
                      TRACE("SimpleCycle");
                      color_applicators.emplace_back(
                        in_place_type_t<SimpleCycleColorApplicator>{},
                        std::move(colors)
                      );
                    },
                    [&](cli::ColorParam::Modes::Cycle& cycle){
                      TRACE("Cycle");
                      // TODO cycle.is_recursive and cycle.color_mode
                      color_applicators.emplace_back(
                        in_place_type_t<CycleColorApplicator>{},
                        std::move(colors), cycle.is_looped
                      );
                    },
                    [&](cli::ColorParam::Modes::Hash&){
                      TRACE("Hash");
                      color_applicators.emplace_back(
                        in_place_type_t<HashColorApplicator>{},
                        std::move(colors)
                      );
                    },
                    [&](cli::ColorParam::Modes::Random& rand){
                      TRACE("Random");
                      color_applicators.emplace_back(
                        in_place_type_t<RandomColorApplicator>{},
                        std::move(colors),
                        rand.seed < 0
                          ? std::random_device{}()
                          : unsigned(rand.seed)
                      );
                    },
                    [&](cli::ColorParam::Modes::Scale& scale){
                      TRACE("Scale");
                      // TODO other members
                      color_applicators.emplace_back(
                        in_place_type_t<SimpleScaleColorApplicator>{},
                        std::move(colors), scale.scale_min, scale.scale_max
                      );
                    }
                  ));
                }
              }
            ));
          }

          mk_maybe_loop(in_place_type_t<Pattern>{}, [&](auto t){
            elems.emplace_back(
              t,
              bctx,
              std::move(reg),
              std::move(color_applicators),
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
        TRACE(" -> ", elems.size() + 1u);
        mk_maybe_loop(in_place_type_t<Group<Jump>>{}, [&](auto t){
          elems.emplace_back(t, bctx, ColoutIndex(elems.size() + 1u));
        });
      }
      else if (param.go_id != -1)
      {
        TRACE(" -> ", param.go_id);
        if (bool(F::call_label | param.activated_flags))
        {
          mk_maybe_loop(in_place_type_t<Call>{}, [&](auto t){
            elems.emplace_back(t, bctx, ColoutIndex(param.go_id));
          });
        }
        else
        {
          mk_maybe_loop(in_place_type_t<Jump>{}, [&](auto t){
            elems.emplace_back(t, ColoutIndex(param.go_id));
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
    colout::ColoutIndex i = colout::zeroAsIndex;
    colout::string_view sv{s.data(), s.size()};
    do
    {
      auto ret = colout::run_at(scanner, i, ctx, sv, 0);
      sv.remove_prefix(ret.countConsumed);
      i = ret.nextId;
    } while (i != colout::invalidIndex);

    std::cout << ctx;
    std::cout.write(sv.data(), sv.size()) << '\n';
    ctx.clear();
  }
}
