#pragma once 
#include "logistics_export.h"

#include "priority.h"
#include <set>
#include <list>
#include <map>
#include <string>
#include <variant>
#include <entt/entity/registry.hpp>
#include <cereal/access.hpp>

typedef unsigned int timing_t;
#define DEFAULT_TIMING_DELTA 1

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
  using diff_t = parameter;

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


//integrating this to attributes_info (with accessor functions) would be more work, but would allow much easier contradicting change detection.
//so maybe future TO-DO
enum class smt {
  added,
  removed
};

struct status_t {
  bool Value;
  using diff_t = smt;
  bool operator==(const status_t &rhs) const{
    return Value == rhs.Value;
  }
  bool operator==(bool rhs) const {
    return Value == rhs;
  }
};

template<typename T>
struct Change {
  typename T::diff_t Diff;
  entt::entity CommitterId;
};

template<typename T>
struct CommittedValue {
  T Value;
  entt::entity CommitterId;
};

struct attributes_info_snapshot {
  std::map<entt::id_type, CommittedValue<status_t>> StatusHashes;
  std::map<entt::id_type, CommittedValue<parameter>> ParamValues;
};



/*struct attributes_info_reference {
  attributes_info_reference(std::set<entt::id_type> &stat, std::map<entt::id_type, parameter> &params) : StatusHashes(stat), ParamValues(params) {}
  attributes_info_reference(attributes_info_snapshot &snapshot) : StatusHashes(snapshot.StatusHashes), ParamValues(snapshot.ParamValues) {}
  std::set<entt::id_type> &StatusHashes;
  std::map<entt::id_type, parameter> &ParamValues;
};*/


//used by the client to know what has changed -- used in trigger filters and functions
struct attributes_info_changes{ 
  std::map<entt::id_type, Change<status_t>> ModifiedStatuses; 
  std::map<entt::id_type, std::pair<CommittedValue<parameter>, CommittedValue<parameter>>> ModifiedParams;
};

//sent to the backend by the clients, who should not have to care about committer IDs
struct logistics_API  attributes_info_short_changes {
  std::map<entt::id_type, smt> ModifiedStatuses; 
  std::map<entt::id_type, parameter> ModifiedParams;
};

struct attributes_info_cumulative_changes {
  std::map<entt::id_type, Change<status_t>> StatusesChanges; 
  std::map<entt::id_type, Change<parameter>> ParamChanges;
};

/*struct attributes_info_state_at_timing {
  attributes_info_short_changes Changes;
  entt::entity OriginatingEntity;
};*/

/*namespace logistics{
  class change_merger;
}*/

template<typename T>
  class diff_merger {
    public:
    //If we are doing cumulative changes : left will have a lower timing than right and right will always be prioritized in case of conflict
    //Otherwise this will be called in add_changes
    virtual Change<T> merge_changes(const Change<T> &left, const Change<T> &right, const priority_callback_t &prio_func) {
      //most basic generic merge function here
      priority_request req;
      priority_t left_prio = 0;
      priority_t right_prio = 0;
      req.EntitiesWithResultingPriorityValues.push_back(std::make_pair(left.CommitterId, &left_prio));
      req.EntitiesWithResultingPriorityValues.push_back(std::make_pair(right.CommitterId, &right_prio));
      prio_func.Func(req, prio_func.UserData);
      
      Change<T> result;
      result.Diff = left_prio > right_prio ? left.Diff : right.Diff;
      result.CommitterId = left_prio > right_prio ? left.CommitterId : right.CommitterId;

      return result;
    }

    //For params : specialize the template to take into account if the diff is incremental (i.e. an HP param for instance)
  };

template<typename T>
struct generic_history {
  std::map<timing_t, Change<T>> History;
  void add_change(timing_t timing, const Change<T> &change, const priority_callback_t &prio_func);
  Change<T> cumulative_change(timing_t upper_bound, const priority_callback_t &prio_func) const;
};

template<typename T>
void generic_history<T>::add_change(timing_t timing, const Change<T> &change, const priority_callback_t &prio_func){
  auto it = History.find(timing);
  if(it == History.end()){
    History.emplace(timing, change);
  } else {
    Change<T> &existing = it->second;
    diff_merger<T> merger;
    existing = merger.merge_changes(existing, change, prio_func);
  }
}

template<typename T>
Change<T> generic_history<T>::cumulative_change(timing_t upper_bound, const priority_callback_t &prio_func) const {
  Change<T> result;
  result.CommitterId = entt::null;
  diff_merger<T> merger;

  auto it = History.begin();
  while(it != History.end() && it->first <= upper_bound){
    //merge because changes may become incremental
    result = merger.merge_changes(result, it->second, prio_func);
    it++;
  }
  
  return result;
}

struct attributes_info_history {
  const attributes_info_snapshot StartingPoint;
  //std::map<timing_t, attributes_info_state_at_timing> History;

  std::map<entt::id_type, generic_history<status_t>> StatusesHistory2; //DA FUTURE
  std::map<entt::id_type, generic_history<parameter>> ParamsHistory2;

  //bool add_changes(timing_t timing, const attributes_info_state_at_timing &changes, const priority_callback_t &callback, void *cb_user_data);
  bool add_changes(timing_t timing, const attributes_info_cumulative_changes &changes, const priority_callback_t &callback);
  //only one commiter per add_changes call???
  //unless short_changes also contains commiter info???
  //to be ascertained in refactor Volume II

  attributes_info_snapshot produce_snapshot(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  attributes_info_cumulative_changes cumulative_changes(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  //bool cumulative_changes_disregarding_timing(attributes_info_short_changes &cumul_changes, timing_t lower_bound, timing_t upper_bound) const;
  //bool cumulative_changes_swiss_knife(attributes_info_short_changes &cumul_changes, timing_t lower_bound, timing_t upper_bound, logistics::change_merger *merger) const;
};


template<typename T>
void populate_change_map(std::map<entt::id_type, Change<T>> &map, const std::map<entt::id_type, typename T::diff_t> &diff_map, entt::entity originating_entity) {
  for(const auto &[hash, diff] : diff_map){
    map.emplace(hash, Change<T>{diff, originating_entity});
  }
}

logistics_API attributes_info_cumulative_changes cumul_changes_from_short(const attributes_info_short_changes &short_changes, entt::entity originating_entity);
logistics_API attributes_info_cumulative_changes cumul_changes_from_long(const attributes_info_changes &changes);
logistics_API bool paste_cumulative_changes(const attributes_info_cumulative_changes &changes, attributes_info_snapshot &snapshot);


logistics_API bool changes_empty(attributes_info_changes &changes);
logistics_API attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot);

enum class changes_category {
  current,
  intrinsics,
  local //during status effects application
};