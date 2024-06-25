#pragma once

#include <entt/entity/registry.hpp>
#include <set>
#include <list>

struct type_inheritance_node {
  type_inheritance_node *Parent{nullptr};
  std::list<type_inheritance_node *> Children;
  std::string Name;
  entt::id_type Hash;
};

struct type_inheritance_graph { //context will have one single instance of this
  std::list<type_inheritance_node> Nodes;
  type_inheritance_node *Root{nullptr};
};

struct attributes_info {
  std::set<entt::id_type> CurrentHashes;
};

bool assign_status(entt::registry &registry, entt::entity entity, const std::string &type_name);
bool assign_status(entt::registry &registry, entt::entity entity, entt::id_type type_hash);

enum class data_type {
  string,
  number,
  integer,
  boolean
};

struct parameter {
  data_type DT;
  std::string Value;
};

bool add_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);

//Now : Modifiers : the revenge
//they should always be additive