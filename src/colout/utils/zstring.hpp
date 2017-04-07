#pragma once

#include <iosfwd>
#include <cassert>
#include <cstring>
#include <utility>

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

  void clear()
  {
    s = nullptr;
  }

private:
  char const * s = nullptr;
};

inline bool operator==(zstring const & a, zstring const & b) noexcept
{
  return strcmp(a.c_str(), b.c_str()) == 0;
}

inline bool operator<(zstring const & a, zstring const & b) noexcept
{
  return strcmp(a.c_str(), b.c_str()) < 0;
}

using namespace std::rel_ops;

template<class Ch, class Tr>
std::basic_ostream<Ch, Tr> &
operator << (std::basic_ostream<Ch, Tr> & os, zstring const & zstr)
{
  return os << (bool(zstr) ? zstr.c_str() : "(nil)");
}

}
