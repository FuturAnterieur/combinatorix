#pragma once 
#include "logistics_export.h"

#include <set>
#include <list>
#include <map>
#include <string>
#include <variant>
#include <entt/entity/fwd.hpp>


enum class data_type {
  null,
  string,
  number,
  integer,
  boolean
};

using parameter_value_t = std::variant<bool, int64_t, float, std::string>;

struct logistics_API parameter {
  parameter();
  parameter(const std::string &val);
  parameter(const char *val);
  parameter(bool val);
  parameter(float val);
  parameter(int64_t val);
  parameter(const parameter &other);
  parameter(parameter &&other);

  parameter &operator=(const parameter &lhs);
  parameter &operator=(parameter &&lhs);

  virtual ~parameter();
  
  const parameter_value_t &value() const;
  data_type dt() const;
  void set_value(parameter_value_t value);
  data_type &access_data_type(); 
  parameter_value_t &access_value();

private:
  data_type DT{data_type::null};
  struct pimpl;
  pimpl *_Pimpl;
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

struct attributes_info_reference {
  std::set<entt::id_type> &StatusHashes;
  std::map<entt::id_type, parameter> &ParamValues;
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

//Isolates what is New in new_changes compared to old_changes
attributes_info_changes compute_changes_diff(const attributes_info_changes &old_changes, const attributes_info_changes &new_changes);