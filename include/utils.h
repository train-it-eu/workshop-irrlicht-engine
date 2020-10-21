/*
 * Copyright (c) 2019, Train IT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <array>
#include <deque>
#include <string>
#include <typeinfo>
#include <vector>

namespace workshop {

/**
 * Class that cannot be copied
 */
class noncopyable {
public:
  noncopyable() = default;
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
};

/**
 * Stores the count of special class operations done on registered types.
 *
 * @note Singleton design pattern
 */
class counters : noncopyable {
public:
  enum {
    constructions,
    copy_constructions,
    move_constructions,
    destructions,
    copy_assignments,
    move_assignments,

    last = move_assignments
  };
  static constexpr int num = last + 1;
  using data = std::array<int, num>;

  static counters& instance();

  data& add(const std::string& name);
  bool validate() const;
  void print(bool detailed) const;

private:
  std::deque<data> data_;
  std::vector<std::string> names_;
  counters() = default;
};

/**
 * Counts special class operations done on specific types.
 *
 * @note CRTP design pattern
 */
template<typename T>
class type_counters {
  static counters::data& counters_;

public:
  // clang-format off
  type_counters()                                 { ++counters_[counters::constructions]; }
  type_counters(const type_counters&)             { ++counters_[counters::copy_constructions]; }
  type_counters(type_counters&&)                  { ++counters_[counters::move_constructions]; }
  ~type_counters()                                { ++counters_[counters::destructions]; }
  type_counters& operator=(const type_counters&)  { ++counters_[counters::copy_assignments]; return *this; }
  type_counters& operator=(type_counters&&)       { ++counters_[counters::move_assignments]; return *this; }
  // clang-format on

  // needed to allow `= default` comparison operators generation in `T`
  bool operator<=>(const type_counters&) const = default;
};

template<class T>
inline counters::data& type_counters<T>::counters_ = counters::instance().add(typeid(T).name());

}  // namespace workshop
