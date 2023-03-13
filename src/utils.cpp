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

#include <irrlicht-engine/utils.h>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>

#ifndef _MSC_VER
#include <cxxabi.h>
#include <memory>
#endif

/* ********************************* S T A T I S T I C S ********************************* */

workshop::counters& workshop::counters::instance()
{
  static counters instance;
  return instance;
}

namespace {

#if _MSC_VER

std::string& demangle(const std::string& name) { return name; }

#else

template<typename T>
struct free_deleter {
  void operator()(T* ptr) { free(ptr); }
};

template<typename T>
using c_ptr = std::unique_ptr<T, free_deleter<T>>;

std::string demangle(const std::string& mangled)
{
  int status;
  c_ptr<char> name(abi::__cxa_demangle(mangled.c_str(), 0, 0, &status));
  return status == 0 ? name.get() : mangled;
}

#endif

}  // namespace

workshop::counters::data& workshop::counters::add(const std::string& name)
{
  data_.emplace_back();
  names_.emplace_back(demangle(name));
  return data_.back();
}

bool workshop::counters::validate() const
{
  assert(data_.size() == names_.size());

  bool problemFound = false;

  std::cout << "\n";

  for (size_t i = 0; i < data_.size(); ++i) {
    const auto& stats = data_[i];
    const auto balance =
      stats[constructions] + stats[copy_constructions] + stats[move_constructions] - stats[destructions];
    problemFound |= (balance != 0);
    if (balance > 0)
      std::cerr << "!!! ERROR !!! " << balance << " memory leaks found in " << names_[i] << ".\n";
    else if (balance < 0)
      std::cerr << "!!! ERROR !!! " << balance << " multiple frees found in " << names_[i] << ".\n";
  }

  if (!problemFound) std::cout << "!NICE WORK! No memory management problems found.\n";

  return !problemFound;
}

void workshop::counters::print(bool detailed) const
{
  assert(data_.size() == names_.size());

  const char* txt[] = {"Constructions", "Copy-constructions", "Move-constructions",
                       "Destructions",  "Copy-assignments",   "Move-assignments"};
  static_assert(std::size(txt) == num, "Statistics descriptions table out of sync!");

  if (detailed) {
    std::cout << "\nDetailed statistics:\n";
    std::cout << "====================\n";
  }

  data overall{};
  for (size_t i = 0; i < data_.size(); ++i) {
    if (detailed) std::cout << " - " << names_[i] << ":\n";
    for (size_t j = 0; j < data_[i].size(); ++j) {
      if (detailed) std::cout << "   " << std::setw(20) << std::left << txt[j] << " = " << data_[i][j] << "\n";
      overall[j] += data_[i][j];
    }
  }

  std::cout << "\nOverall statistics:\n";
  std::cout << "===================\n";
  for (size_t i = 0; i < overall.size(); ++i)
    std::cout << "   " << std::setw(20) << std::left << txt[i] << " = " << overall[i] << "\n";
}
