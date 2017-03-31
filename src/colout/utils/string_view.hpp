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
* \brief     string_view
*/

#pragma once

#include <cstdint>
#include <cstring> // strlen
#include <cassert>


namespace colout
{
  struct string_view
  {
    using iterator = char const *;
    using const_iterator = iterator;

    constexpr string_view() = default;
    constexpr string_view(std::nullptr_t) = delete;

    constexpr string_view(char const * s, char const * send) noexcept
    : s_(s)
    , n_(std::size_t(send - s))
    {
      assert(s && send);
      assert(s <= send);
    }

    constexpr string_view(char const * s, std::size_t n) noexcept
    : s_(s)
    , n_(n)
    {
    }

    string_view(char const * s) noexcept
    : s_(s)
    , n_(strlen(s))
    {
      assert(s);
    }

    constexpr string_view
    substr(std::size_t pos) const
    {
      assert(pos <= n_);
      return string_view{s_ + pos, s_ + n_};
    }

    constexpr char operator[](std::size_t i) const noexcept { return s_[i]; }
    constexpr std::size_t size() const noexcept { return n_; }
    constexpr bool empty() const noexcept { return !n_; }
    constexpr char const * data() const noexcept { return s_; }

    const_iterator begin() const noexcept { return s_; }
    const_iterator end() const noexcept { return s_ + n_; }

    void remove_prefix(std::size_t n) noexcept
    {
      assert(n <= n_);
      s_ += n;
      n_ -= n;
    }

    void remove_suffix(std::size_t n) noexcept
    {
      assert(n <= n_);
      n_ -= n;
    }

  private:
    char const * s_ = nullptr;
    std::size_t n_ = 0;
  };

  inline constexpr string_view operator
  "" _sv(char const * s, std::size_t n)
  { return {s, n}; }
}
