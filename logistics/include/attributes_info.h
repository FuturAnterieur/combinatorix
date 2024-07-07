#pragma once 
#include "logistics_export.h"

#include "change_merger.h"
#include <set>
#include <list>
#include <map>
#include <string>
#include <variant>
#include <entt/entity/fwd.hpp>
#include <cereal/access.hpp>

typedef unsigned int timing_t;
#define DEFAULT_TIMING_DELTA 1
typedef size_t priority_t;


enum class data_type {
  null,
  string,
  number,
  integer,
  boolean
};

using parameter_value_t = std::variant<bool, int64_t, float, std::string>;

template<typename T>
struct concrete_to_enum_type{static constexpr data_type t;};

template<>
struct concrete_to_enum_type<bool>{static constexpr data_type t = data_type::boolean;};

template<>
struct concrete_to_enum_type<std::string>{static constexpr data_type t = data_type::string;};


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
  
private:
  friend class cereal::access;
  data_type &access_data_type(); 
  parameter_value_t &access_value();

  template<typename Archive>
  void serialize(Archive &archive){
    archive(access_data_type(), access_value());
  }

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
  attributes_info_reference(std::set<entt::id_type> &stat, std::map<entt::id_type, parameter> &params) : StatusHashes(stat), ParamValues(params) {}
  attributes_info_reference(attributes_info_snapshot &snapshot) : StatusHashes(snapshot.StatusHashes), ParamValues(snapshot.ParamValues) {}
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
  timing_t Timing{0}; //Used only sparingly for now.
  priority_t Priority{0}; //Used only sparingly for now. This is not the final priority mechanic. The real one should be based on customizable game rules.
};

bool paste_attributes_changes(const attributes_info_changes &changes, attributes_info_reference &ref);

struct attributes_info_state_at_timing {
  attributes_info_changes Changes;
  //attributes_info_snapshot Snapshot;
};

struct attributes_info_history {
  const attributes_info_snapshot StartingPoint;
  std::map<timing_t, attributes_info_state_at_timing> History;
  bool add_changes(timing_t timing, const attributes_info_changes &changes);
  attributes_info_snapshot produce_snapshot(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  logistics::merge_result cumulative_changes(attributes_info_changes &cumul_changes, timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
};

bool changes_empty(attributes_info_changes &changes);
attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot);

//Isolates what is New in new_changes compared to old_changes
attributes_info_changes compute_changes_diff(const attributes_info_changes &old_changes, const attributes_info_changes &new_changes);

enum class changes_category {
  current,
  intrinsics,
  local //during status effects application
};