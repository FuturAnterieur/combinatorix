#pragma once

#include <entt/entity/registry.hpp>
#include <vector>

struct type_inheritance_graph { //context will have one single instance of this

};

struct has_type {
  std::vector<uint32_t> type_ids;
};
