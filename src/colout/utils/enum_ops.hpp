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

#include <type_traits>

namespace colout
{
  template<class T>
  using underlying_type_t = typename std::underlying_type<T>::type;

  template<class E>
  constexpr
  underlying_type_t<E>
  underlying_cast(E e)
  { return static_cast<underlying_type_t<E>>(e); }


  namespace detail_
  {
      template<bool>
      struct enable_if_impl
      {
          template<class T> using type = T;
      };

      template<>
      struct enable_if_impl<false>;
  }

  template<class T>
  struct is_enum_flags : std::false_type
  {};

  template<class T>
  using enable_if_enum_flags
    = typename detail_::enable_if_impl<is_enum_flags<T>::value>
    ::template type<T>;

}

template<class T>
constexpr
colout::enable_if_enum_flags<T>
operator ~ (T x) noexcept
{ return T(~colout::underlying_cast(x)); }

template<class T>
constexpr
colout::enable_if_enum_flags<T>
operator & (T x, T y) noexcept
{ return T(colout::underlying_cast(x) & colout::underlying_cast(y)); }

template<class T>
constexpr
colout::enable_if_enum_flags<T>
operator | (T x, T y) noexcept
{ return T(colout::underlying_cast(x) | colout::underlying_cast(y)); }

template<class T>
/*FALCON_CXX14_CONSTEXPR*/
colout::enable_if_enum_flags<T> &
operator &= (T & x, T y) noexcept
{ return x = T(colout::underlying_cast(x) & colout::underlying_cast(y)); }

template<class T>
/*FALCON_CXX14_CONSTEXPR*/
colout::enable_if_enum_flags<T> &
operator |= (T & x, T y) noexcept
{ return x = T(colout::underlying_cast(x) | colout::underlying_cast(y)); }
