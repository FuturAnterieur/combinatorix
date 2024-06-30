#pragma once

#include <functional>

struct base_trigger {
  std::function<bool()> Filter;
  std::function<void()> Func;
};