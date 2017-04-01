#include "colout/cli/parse_cli.hpp"
#include "colout/utils/fixed_array.hpp"

#include <falcon/cxx/cxx.hpp>

#include <type_traits>

#include <iostream>
#include <iterator>


using std::begin;
using std::end;

namespace colout
{
  constexpr string_view esc_reset
    FALCON_IN_IDE_PARSER_CONDITIONAL(FALCON_PP_NIL, = "\033[0m"_sv);

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

  struct ColorGen
  {
    ColorGen(std::vector<colout::Color> colors, bool loop)
    : mColors(colors)
    , mMaxColor(int(mColors.size())-1)
    , mModColor(loop ? int(mColors.size()) : std::numeric_limits<int>::max())
    {
    }

    colout::Color const & next_color()
    {
      auto i = std::min(mIColor % mModColor, mMaxColor);
      auto const & color = mColors[i];
      ++mIColor;
      return color;
    }

    void reset()
    {
      mIColor = 0;
    }

  private:
    std::vector<colout::Color> mColors;
    int mMaxColor;
    int mModColor;
    int mIColor = 0;
  };

  struct Scanner;

  using F = cli::ActiveFlags;

  struct Pattern
  {
    Pattern(
      BranchCtx bctx,
      std::regex reg,
      std::vector<colout::Color> colors,
      F f
    )
    : mReg(std::move(reg))
    , mColorGen(std::move(colors), bool(F::loop_color & f))
    , mEscReset(bool(F::no_reset & f) ? string_view{} : esc_reset)
    , mResetColor(!bool(F::keep_color & f))
    , mContinueFromLastColor(bool(F::continue_from_last_color & f))
    , mEndColorMark(bool(F::end_color_mark & f))
    , mBCtx(bctx)
    {}

    VisitorResult run(Scanner&, std::string& ctx, string_view sv)
    {
      std::size_t pos = 0;
      bool const exists = std::regex_search(sv.begin(), sv.end(), mMatch, mReg);

      if (exists)
      {
        if (!mReg.mark_count())
        {
          pos = push_color(ctx, 0, pos, sv);
        }
        else
        {
          for (size_t i = 1; i < mMatch.size(); ++i)
          {
            pos = push_color(ctx, i, pos, sv);
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
          auto const & color = mColorGen.next_color();
          ctx.append(begin(color), end(color));
        }

        if (mResetColor)
        {
          mColorGen.reset();
        }
      }

      return VisitorResult{exists, mBCtx.computeNextId(exists), pos};
    }

  private:
    std::size_t push_color(
      std::string& ctx, size_t i, std::size_t pos, string_view sv
    ) {
      auto const new_pos = mMatch.position(i);
      auto const len = mMatch.length(i);
      auto const & color = mColorGen.next_color();
      ctx
        .append(begin(sv) + pos,      begin(sv) + new_pos)
        .append(begin(color),         end(color))
        .append(begin(sv) + new_pos,  begin(sv) + new_pos + len)
        .append(begin(mEscReset),     end(mEscReset))
      ;
      return std::size_t(new_pos + len);
    }

    std::regex mReg;
    std::cmatch mMatch;
    ColorGen mColorGen;
    string_view mEscReset;
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

  class Scanner;

  VisitorResult run_at(Scanner& scanner, int id, std::string& ctx, string_view sv);

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
      std::size_t const s_sz = ctx.size();

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
        ctx.resize(s_sz);
        return {false, mBCtx.nextIdFail, 0};
      }
    }

  private:
    Pattern mPattern;
    BranchCtx mBCtx;
  };

  struct Jump
  {
    Jump(std::size_t i)
    : mI(int(i))
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
    Call(BranchCtx bctx, std::size_t i)
    : mI(int(i))
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
    TestPattern(std::size_t i, std::regex reg)
    : mReg(std::move(reg))
    , mI(int(i))
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
    TestPatternAndCall(BranchCtx bctx, std::size_t i, std::regex reg)
    : mReg(std::move(reg))
    , mI(int(i))
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


  struct Labels
  {
    using Pair = std::pair<zstring, std::size_t>;
    struct Less
    {
      bool operator()(Pair const & a, Pair const & b) const
      {
        return a.first < b.first;
      };
    };

    Labels(std::vector<cli::ColoutParam> const & coloutParams)
    {
      for (std::size_t i = 0; i < coloutParams.size(); ++i)
      {
        if (zstring const label{coloutParams[i].label})
        {
          labels.emplace_back(label, i);
        }
      }

      std::sort(labels.begin(), labels.end(), Less{});

      auto cmp = [](auto& a, auto& b){ return a.first == b.first; };
      auto p = std::adjacent_find(labels.begin(), labels.end(), cmp);
      if (labels.end() != p)
      {
        throw cli::runtime_cli_error(
          std::string("Duplicated label ") + p->first.c_str()
        );
      }
    }

    std::size_t find(zstring label) const
    {
      auto p = std::lower_bound(
        labels.begin(), labels.end(),
        Pair{label, 0}, Less{}
      );
      if (p == labels.end() || p->first != label)
      {
        throw cli::runtime_cli_error(
          std::string("Unknown label ") + label.c_str()
        );
      }
      return p->second;
    }

  private:
    std::vector<Pair> labels;
  };

  template<class T>
  struct PlaceType
  {
    explicit PlaceType() = default;
    using type = T;
  };

  template<class Ints, class... Ts>
  struct indexedType;

  template<class i, class T>
  struct pairType {};

  template<std::size_t... I, class... Ts>
  struct indexedType<std::integer_sequence<std::size_t, I...>, Ts...>
  : pairType<std::integral_constant<std::size_t, I>, Ts>...
  {};

  template<class T, class I>
  I get_index_of(pairType<I, T>*);

  template<class T, class... Ts>
  using indexOf = decltype(get_index_of<T>(
    static_cast<
      indexedType<
        std::index_sequence_for<Ts...>,
        Ts...
      >*
    >(nullptr)
  ));

  template<class R, class T, class F>
  struct PtrFuncHelper
  {
    static R impl(F&& f, void * d)
    {
      return f(*static_cast<T*>(d));
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

    Variant(Variant const &) = delete;
    Variant operator=(Variant const &) = delete;

    ~Variant()
    {
      visit([](auto & d) {
        using D = std::remove_reference_t<decltype(d)>;
        d.~D();
      });
    }

    template<class F, class R = decltype(std::declval<F&&>()(std::declval<T&>()))>
    R visit(F && f)
    {
      using proto = R(*)(F &&, void *);
      static constexpr proto func_ptrs[]{
        PtrFuncHelper<R, T, F>::impl,
        PtrFuncHelper<R, Ts, F>::impl...
      };
      return func_ptrs[idx_](std::forward<F>(f), &data_);
    }

    typename std::aligned_union_t<0, T, Ts...>::type data_;
    int idx_;
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
      using Variant::Variant;
    };

    fixed_array<Element> elems;
  };

  VisitorResult run_at(Scanner& scanner, int id, std::string& ctx, string_view sv)
  {
    assert(id != -1);
    std::cerr << id << " " << sv << "\n";
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
    Labels labels{params};

    fixed_array<Scanner::Element> elems(params.size());

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
            elems.emplace_back(t, bctx, elems.size() + 1u, std::move(reg));
          });
        }
        else if (param.go_label)
        {
          auto const jumpId = labels.find(param.go_label);
          if (bool(F::call_label | param.activated_flags))
          {
            mk_maybe_loop(PlaceType<TestPatternAndCall>{}, [&](auto t){
              elems.emplace_back(t, bctx, jumpId, std::move(reg));
            });
          }
          else
          {
            mk_maybe_loop(PlaceType<TestPattern>{}, [&](auto t){
              elems.emplace_back(t, jumpId, std::move(reg));
            });
          }
        }
        else
        {
          mk_maybe_loop(PlaceType<Pattern>{}, [&](auto t){
            elems.emplace_back(
              t,
              bctx,
              std::move(reg),
              std::move(param.colors),
              param.activated_flags
            );
          });
        }
      }
      else if (bool(F::start_group & param.activated_flags))
      {
        mk_maybe_loop(PlaceType<Group<Jump>>{}, [&](auto t){
          elems.emplace_back(t, bctx, elems.size() + 1u);
        });
      }
      else if (param.go_label)
      {
        auto const jumpId = labels.find(param.go_label);
        if (bool(F::call_label | param.activated_flags))
        {
          mk_maybe_loop(PlaceType<Call>{}, [&](auto t){
            elems.emplace_back(t, bctx, jumpId);
          });
        }
        else
        {
          mk_maybe_loop(PlaceType<Jump>{}, [&](auto t){
            elems.emplace_back(t, jumpId);
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
      << "  r(" << bool(colout_param.activated_flags & F::regex_flags) << "): "
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
