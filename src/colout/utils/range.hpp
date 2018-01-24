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

#include <iterator>
#include <cassert>


namespace colout
{
  template<class T>
  struct IntIterator
  {
    T value;

    constexpr T const & operator*() { return value; }
    constexpr T const * operator->() { return &value; }

    constexpr IntIterator & operator ++() { ++value; return *this; }

    constexpr bool operator==(IntIterator const & other) const { return value == other.value; }
    constexpr bool operator!=(IntIterator const & other) const { return !(*this == other); }
    constexpr bool operator<(IntIterator const & other) const { return value < other.value; }
  };

  template<class It>
  struct Range
  {
    It first_;
    It last_;

    using value_type = typename std::iterator_traits<It>::value_type;
    using reference = typename std::iterator_traits<It>::reference;
    using const_reference = value_type const &;
    using difference_type = typename std::iterator_traits<It>::difference_type;

    reference front() { assert(!empty()); return *first_; }
    const_reference front() const { assert(!empty()); return *first_; }

    reference back() { assert(!empty()); return *(last_-1); }
    const_reference back() const { assert(!empty()); return *(last_-1); }

    constexpr value_type const & operator[](std::size_t i) const noexcept { return first_[i]; }
    constexpr difference_type size() const noexcept { return last_ - first_; }
    constexpr bool empty() const noexcept { return last_ == first_; }
    constexpr It begin() const noexcept { return first_; }
    constexpr It end() const noexcept { return last_; }
  };

  template<class T>
  constexpr Range<IntIterator<T>>
  range(T start, T stop)
  { return {{start}, {stop}}; }

  template<class T, class U>
  constexpr auto range(T start, U stop)
  -> Range<IntIterator<decltype(1 ? start : stop)>>
  {
    using int_type = decltype(1 ? start : stop);
    return {{static_cast<int_type>(start)}, {static_cast<int_type>(stop)}}; }
}

namespace std
{
  template<class T>
  struct iterator_traits<colout::IntIterator<T>>
  : iterator_traits<T*>
  {};
}
