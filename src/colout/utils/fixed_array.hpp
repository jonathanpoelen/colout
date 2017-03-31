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

#pragma once

#include <cstdint>
#include <cassert>
#include <utility>
#include <new>

// TODO limited_array
template<class T>
struct fixed_array
{
  fixed_array(std::size_t n)
  : m_elems(static_cast<T*>(::operator new[](sizeof(T) * n)))
#ifndef NDEBUG
  , m_sz(n)
#endif
  {}

  fixed_array(fixed_array && other) noexcept
  : m_elems(std::exchange(other.m_elems, nullptr))
  , m_i(std::exchange(other.m_i, 0))
#ifndef NDEBUG
  , m_sz(std::exchange(other.m_sz, 0))
#endif
  {}

  fixed_array(fixed_array const&) = delete;

  ~fixed_array()
  {
    while (m_i)
    {
      m_elems[--m_i].~T();
    }
    ::operator delete[](m_elems);
  }

  fixed_array& operator=(fixed_array && other) noexcept
  {
    m_elems = std::exchange(other.m_elems, nullptr);
    m_i = std::exchange(other.m_i, 0);
#ifndef NDEBUG
    m_sz = std::exchange(other.m_sz, 0);
#endif
    return *this;
  }

  fixed_array& operator=(fixed_array const&) = delete;

  T& operator[](std::size_t i)
  {
    assert(i < m_i);
    return m_elems[i];
  }

  T const& operator[](std::size_t i) const
  {
    assert(i < m_i);
    return m_elems[i];
  }

  std::size_t size() const { return m_i; }

  T* begin() { return m_elems; }
  T* end() { return m_elems + m_i; }
  T const* begin() const { return m_elems; }
  T const* end() const { return m_elems + m_i; }

  template<class... Ts>
  T& emplace_back(Ts&&... args)
  {
    assert(m_i != m_sz);
    new (m_elems + m_i) T(std::forward<Ts>(args)...);
    return m_elems[m_i++];
  }

private:
  T * m_elems;
  std::size_t m_i = 0;
#ifndef NDEBUG
  std::size_t m_sz;
#endif
};
