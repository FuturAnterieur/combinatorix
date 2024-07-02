#pragma once 

#include <set>
#include <list>
#include <map>
#include <string>
#include <entt/entity/fwd.hpp>

enum class data_type {
  null,
  string,
  number,
  integer,
  boolean
};

struct parameter {
  data_type DT{data_type::null};
  std::string Value{};
};

struct attributes_info {
  std::set<entt::id_type> IntrinsicStatusHashes;
  std::set<entt::id_type> CurrentStatusHashes;
  std::map<entt::id_type, parameter> IntrinsicParamValues;
  std::map<entt::id_type, parameter> CurrentParamValues;
};


struct attributes_info_snapshot {
  std::set<entt::id_type> StatusHashes;
  std::map<entt::id_type, parameter> ParamValues;
};

//integrating this to attributes_info (with accessor functions) would be more work, but would allow much easier contradicting change detection.
//so maybe future TO-DO
enum class smt {
  added,
  removed
};

struct attributes_info_changes{ 
  std::map<entt::id_type, smt> ModifiedStatuses; 
  std::map<entt::id_type, std::pair<parameter, parameter>> ModifiedParams;
};

bool changes_empty(attributes_info_changes &changes);
attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot);