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

#include "colout/utils/string_view.hpp"
#include "colout/utils/range.hpp"

namespace colout
{
  struct Splitter
  {
    using range = Range<char const *>;

    Splitter(range str, char sep) noexcept
    : first_(str.begin())
    , last_(str.end())
    , cur_(first_)
    , sep_(sep)
    {}

    string_view next() noexcept
    {
      this->first_ = this->cur_;
      while (this->cur_ != last_ && *this->cur_ != this->sep_ ) {
        ++this->cur_;
      }
      string_view sv{this->first_, this->cur_};
      if (this->cur_ != this->last_ ) {
        ++this->cur_;
      }
      return sv;
    }

    bool empty() const noexcept
    {
      return this->first_ == this->last_;
    }

  private:
    class iterator
    {
      Splitter & Splitter_;
      string_view sv_;

      friend class Splitter;

      iterator(Splitter & s) noexcept
      : Splitter_(s)
      , sv_(s.next())
      {}

      iterator(Splitter & s, int) noexcept
      : Splitter_(s)
      {}

    public:
      iterator& operator++() noexcept
      {
        this->sv_ = this->Splitter_.next();
        return *this;
      }

      const string_view & operator*() const noexcept
      {
        return this->sv_;
      }

      const string_view * operator->() const noexcept
      {
        return &this->sv_;
      }

      bool operator==(iterator const & other) const noexcept
      {
        return this->Splitter_.first_ == other.Splitter_.last_;
      }

      bool operator!=(iterator const & other) const noexcept
      {
        return !this->operator==(other);
      }
    };

  public:
    iterator begin() noexcept { return iterator{*this}; }
    iterator end() noexcept { return iterator{*this, 1}; }

  private:
    char const * first_;
    char const * last_;
    char const * cur_;
    char sep_;
  };


  inline Splitter get_lines(string_view s, char sep = '\n') noexcept
  {
      return {{s.begin(), s.end()}, sep};
  }
}
