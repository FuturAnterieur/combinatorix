#pragma once

#include "logistics_export.h"
#include <entt/entity/registry.hpp>
#include <set>
#include <list>
#include <map>

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


logistics_API bool assign_status(entt::registry &registry, entt::entity entity, const std::string &status_name, bool is_original = true);
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
  std::map<entt::id_type, parameter> CurrentParamValues;
};

struct attributes_info_snapshot {
  std::set<entt::id_type> StatusHashes;
  std::map<entt::id_type, parameter> ParamValues;
};

logistics_API bool add_original_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);
logistics_API bool add_additional_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);

void reset_original_status(entt::registry &registry, attributes_info_snapshot &snapshot, entt::entity entity);

void reset_original_status(entt::registry &registry, attributes_info &entity_attr_info, attributes_info_snapshot &snapshot, entt::entity entity);
void commit_attr_info(entt::registry &registry, attributes_info &attr_info, attributes_info_snapshot &snapshot, entt::entity entity);

//integrating this to attributes_info (with accessor functions) would be more work, but would allow much easier contradicting change detection.
//so maybe future TO-DO
struct attributes_info_changes{ 
  std::set<entt::id_type> AddedStatuses; 
  std::set<entt::id_type> RemovedStatuses;
  std::map<entt::id_type, parameter> AddedParams;
  std::map<entt::id_type, std::pair<parameter, parameter>> ModifiedParams;
  std::map<entt::id_type, parameter> RemovedParams; //contains old value
};

struct on_status_change_trigger_info {
  //registry, status changes, entity whose status changed, owning entity of the trigger
  std::function<void(entt::registry &, const attributes_info_changes &, entt::entity, entt::entity)> Func;
  entt::entity TriggerOwner;
};

struct on_status_change_triggers {
  std::list<on_status_change_trigger_info> Triggers;
};


//Now : Modifiers : the revenge
//they should always be additive