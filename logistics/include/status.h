#pragma once

#include "logistics_export.h"
#include "attributes_info.h"
#include <entt/entity/registry.hpp>

typedef unsigned int timing_t;

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

logistics_API bool assign_active_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool status_value);
logistics_API bool init_intrinsic_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash, bool status_value);
logistics_API bool get_active_value_for_status(entt::registry &registry, entt::entity entity, entt::id_type status_hash);

//This is destined to be called by status modifying functions, so it does not commit the change to the registry
logistics_API bool add_or_set_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);
logistics_API bool init_intrinsic_parameter(entt::registry &registry, entt::entity entity, const std::string &param_name, data_type dt, const std::string &value);
logistics_API parameter get_active_value_for_parameter(entt::registry &registry, entt::entity entity, entt::id_type param_hash);

bool paste_attributes_changes(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes, attributes_info_reference &ref, bool affect_registry = false);

//Outside of simulation for now
//Changing intrinsics from inside a simulation will require better timing management
logistics_API bool assign_intrinsic_attributes_changes(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes);

void activate_status_change_triggers(entt::registry &registry, entt::entity entity, const attributes_info_changes &changes);

void reset_original_status(entt::registry &registry, attributes_info_snapshot &snapshot, entt::entity entity);
void commit_attr_info_to_branch(entt::registry &registry, attributes_info &attr_info, attributes_info_snapshot &snapshot, entt::entity entity);

struct on_status_change_trigger_info;
using status_change_trigger_func_t = std::function<void(entt::registry &, const attributes_info_changes &, entt::entity, const on_status_change_trigger_info &info)>;
using status_change_trigger_filter_t = std::function<bool(entt::registry &, const attributes_info_changes &, entt::entity, const on_status_change_trigger_info &info)>;
struct on_status_change_trigger_info {
  //registry, status changes, entity whose status changed, owning entity of the trigger
  status_change_trigger_func_t Func;
  status_change_trigger_filter_t Filter;
  entt::entity TriggerOwner;
  timing_t TimeDelta{1}; //unsigned! Cannot trigger stuff before the thing that triggers it happens!
};

struct on_status_change_triggers {
  std::list<on_status_change_trigger_info> Triggers;
};

logistics_API void add_on_status_change_trigger(entt::registry &registry, entt::entity entity, on_status_change_trigger_info &info);
logistics_API void add_global_on_status_change_trigger(entt::registry &registry, entt::entity entity, on_status_change_trigger_info &info);
void add_on_status_change_trigger(entt::registry &registry, on_status_change_triggers &triggers, on_status_change_trigger_info &info);

