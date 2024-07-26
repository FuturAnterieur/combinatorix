#pragma once

#include "logistics_export.h"
#include "attributes_info.h"
#include <entt/entity/registry.hpp>


//For now this is a dead header, but it contains ideas that might be useful for DA FUTURE
struct type_inheritance_node {
  type_inheritance_node *Parent{nullptr}; //NO! Could have many parents. To be confirmed.
  std::list<type_inheritance_node *> Children;
  std::string Name;
  entt::id_type Hash;
};

struct type_inheritance_graph { //context will have one single instance of this
  std::list<type_inheritance_node> Nodes;
  type_inheritance_node *Root{nullptr};
};

//TODO : rules triggers - they are entities we can get thru views