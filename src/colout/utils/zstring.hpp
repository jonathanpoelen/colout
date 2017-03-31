#pragma once

#include <iosfwd>
#include <cassert>
#include <cstring>

namespace colout {

struct zstring
{
  zstring() = default;

  constexpr zstring(char const * s) noexcept
  : s(s)
  {}

  constexpr explicit operator bool () const noexcept
  {
    return bool(s);
  }

  constexpr char operator[](unsigned i) const
  {
    assert(s);
    return s[i];
  }

  constexpr zstring substr_at(unsigned index_start) const noexcept
  {
    assert(s);
    return zstring{s + index_start};
  }

  constexpr char const * c_str() const noexcept
  {
    assert(s);
    return s;
  }

  constexpr char const * get() const noexcept
  {
    return s;
  }

private:
  char const * s = nullptr;
};

inline bool operator==(zstring const & a, zstring const & b) noexcept
{
  return 0 == strcmp(a.c_str(), b.c_str());
}

inline std::ostream & operator << (std::ostream & os, zstring const & zstr)
{
  return os << (bool(zstr) ? zstr.c_str() : "(nil)");
}

}
