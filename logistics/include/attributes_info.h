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

template<typename T>
struct CommittedValue {
  T Value;
  entt::entity CommitterId{entt::null};
};

template<typename T>
struct Change {
  typename T::diff_t Diff;
  entt::entity CommitterId{entt::null};
};

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
  using detailed_change_t = std::pair<CommittedValue<parameter>, CommittedValue<parameter>>;

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
  using detailed_change_t = Change<status_t>;
  bool operator==(const status_t &rhs) const{
    return Value == rhs.Value;
  }
  bool operator==(bool rhs) const {
    return Value == rhs;
  }

  status_t &operator=(bool rhs){
    Value = rhs;
    return *this;
  }
};

template<typename T>
struct DetailedChange {
  typename T::detailed_change_t Change;
};

template<typename T>
struct attributes_snapshot_t {
  std::map<entt::id_type, CommittedValue<T>> Values;
};

template<typename T>
struct attributes_changes_t {
  std::map<entt::id_type, Change<T>> Changes;
};

template<typename T>
struct attributes_detailed_changes_t {
  std::map<entt::id_type, DetailedChange<T>> Changes;
};



template<typename T>
inline CommittedValue<T> apply_change(const CommittedValue<T> &orig, const Change<T> &change){
  CommittedValue<T> result;
  result.CommitterId = change.CommitterId;
  result.Value = change.Diff;
  return result;
}

template<>
inline CommittedValue<status_t> apply_change(const CommittedValue<status_t> &orig, const Change<status_t> &change){
  CommittedValue<status_t> result;
  result.CommitterId = change.CommitterId;
  result.Value = (change.Diff == smt::added) ? true : false;
  return result;
}

template<typename T>
Change<T> shorten_change(const DetailedChange<T> &change);

template<>
inline Change<status_t> shorten_change(const DetailedChange<status_t> &change){
  Change<status_t> result;
  result.CommitterId = change.Change.CommitterId;
  result.Diff = change.Change.Diff;
  return result;
}

template<>
inline Change<parameter> shorten_change(const DetailedChange<parameter> &change){
  Change<parameter> result;
  result.CommitterId = change.Change.second.CommitterId;
  result.Diff = change.Change.second.Value;
  return result;
}

template<typename T>
bool compute_diff(const CommittedValue<T> *left, const CommittedValue<T> *right, DetailedChange<T> &result);

template<>
inline bool compute_diff(const CommittedValue<status_t> *left, const CommittedValue<status_t> *right, DetailedChange<status_t> &result){
  bool has_change = false;
  bool left_exists = left && left->Value == true;
  bool right_exists = right && right->Value == true;

  if(left_exists && !right_exists) {
    result.Change.Diff = smt::removed;
    has_change = true;
  } else if (!left_exists && right_exists) {
    result.Change.Diff = smt::added;
    has_change = true;
  }

  if(right){
    result.Change.CommitterId = right->CommitterId;
  }
  return has_change;
}

template<>
inline bool compute_diff(const CommittedValue<parameter> *left, const CommittedValue<parameter> *right, DetailedChange<parameter> &result){
  bool has_change = false;

  result.Change.first = left ? *left : CommittedValue{parameter{}, entt::null};
  result.Change.second = right ? *right : CommittedValue{parameter{}, entt::null};;

  if(result.Change.first.Value.value() == result.Change.second.Value.value()){
    return false;
  }
  
  return true;
}



template<typename T>
inline attributes_detailed_changes_t<T> compute_diff(const attributes_snapshot_t<T> &left, const attributes_snapshot_t<T> &right){
  attributes_detailed_changes_t<T> all_changes;
  std::map<entt::id_type, std::pair<const CommittedValue<T> *, const CommittedValue<T> *>> pairs_map;

  for(const auto &[hash, val] : left.Values){
    pairs_map.emplace(hash, std::make_pair(&val, nullptr));
  }

  for(const auto &[hash, val] : right.Values){
    auto it = pairs_map.find(hash);
    if(it == pairs_map.end()){
      pairs_map.emplace(hash, std::make_pair(nullptr, &val));
    } else {
      it->second.second = &val;
    }
  }

  for(const auto &[hash, pair] : pairs_map){
    DetailedChange<T> result;
    if(compute_diff(pair.first, pair.second, result)){
      all_changes.Changes.emplace(hash, result);
    }
  }

  return all_changes;
}

template<typename T>
inline void apply_changes_to_snapshot(const attributes_changes_t<T> &changes, attributes_snapshot_t<T> &snapshot){
  for(const auto &[hash, change] : changes.Changes){
    auto it = snapshot.Values.find(hash);
    if(it == snapshot.Values.end()){
      snapshot.Values.emplace(hash, apply_change(CommittedValue<T>{}, change));
    } else {
      it->second = apply_change(it->second, change);
    }
  }
}

template<typename T>
inline attributes_changes_t<T> shorten_changes(const attributes_detailed_changes_t<T> &detailed){
  attributes_changes_t<T> shortened;
  for(const auto &[hash, change] : detailed.Changes){
   shortened.Changes.emplace(hash, shorten_change(change));
  }
  return shortened;
}



struct attributes_info_snapshot {
  attributes_snapshot_t<status_t> StatusHashes;
  attributes_snapshot_t<parameter> ParamValues;
};


//used by the client to know what has changed -- used in trigger filters and functions
struct attributes_info_changes{ 
  attributes_detailed_changes_t<status_t> ModifiedStatuses; 
  attributes_detailed_changes_t<parameter> ModifiedParams;
};

//sent to the backend by the clients, who should not have to care about committer IDs
struct logistics_API  attributes_info_short_changes {
  std::map<entt::id_type, smt> ModifiedStatuses; 
  std::map<entt::id_type, parameter> ModifiedParams;
};

struct attributes_info_cumulative_changes {
  attributes_changes_t<status_t> StatusesChanges; 
  attributes_changes_t<parameter> ParamChanges;
};

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
  
  std::map<entt::id_type, generic_history<status_t>> StatusesHistory; //DA FUTURE
  std::map<entt::id_type, generic_history<parameter>> ParamsHistory;

  bool add_changes(timing_t timing, const attributes_info_cumulative_changes &changes, const priority_callback_t &callback);
  
  attributes_info_snapshot produce_snapshot(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  attributes_info_cumulative_changes cumulative_changes(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  
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