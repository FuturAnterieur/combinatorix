#pragma once

#include <entt/entity/registry.hpp>
#include <set>
#include <list>
#include <map>

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


bool assign_status(entt::registry &registry, entt::entity entity, const std::string &status_name, bool is_original = true);
bool assign_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool is_original = true);

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

struct attributes_info {
  std::set<entt::id_type> OriginalStatusHashes;
  std::set<entt::id_type> CurrentStatusHashes;
  std::map<entt::id_type, parameter> OriginalParamValues;
  std::set<entt::id_type> CurrentParamHashes;
};

bool add_original_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);
bool add_additional_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);

void reset_original_status(entt::registry &registry, entt::entity entity);
void reset_all_original_status(entt::registry &registry);

void reset_original_status(entt::registry &registry, attributes_info &attr_info, entt::entity entity);

//Now : Modifiers : the revenge
//they should always be additive