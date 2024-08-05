#pragma once 
#include "logistics_export.h"

#include "priority.h"
#include "numeric_value.h"
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
  not_null,
  string,
  number,
  boolean
};

using parameter_value_t = std::variant<bool, numeric_value, std::string>;

template<typename T>
struct concrete_to_enum_type{static constexpr data_type t;};

template<>
struct concrete_to_enum_type<bool>{static constexpr data_type t = data_type::boolean;};

template<>
struct concrete_to_enum_type<std::string>{static constexpr data_type t = data_type::string;};

enum class param_diff_type {
  set_value,
  apply_op
};

enum class param_op_type {
  add,
  mul,
  mul_then_add
};

struct param_set_value_diff_data{
  parameter_value_t Value;
  bool HasValue{true};
};

struct param_apply_op_diff_data {
  param_op_type OpType;
  std::vector<float> Args;
};

struct parameter_diff {
  param_diff_type Type;
  std::variant<param_set_value_diff_data, param_apply_op_diff_data> Data;
};

logistics_API parameter_diff diff_from_set_val(parameter_value_t val);
logistics_API parameter_diff diff_from_op(param_op_type op, float arg);

struct logistics_API parameter {
  using diff_t = parameter_diff;
  using detailed_change_t = std::pair<CommittedValue<parameter>, CommittedValue<parameter>>;

  parameter();
  parameter(const parameter_value_t &val);
  parameter(const std::string &val);
  parameter(const char *val);
  parameter(bool val);
  parameter(const numeric_value &other);
  parameter(const parameter &other);
  parameter(parameter &&other);

  parameter &operator=(const parameter &lhs);
  parameter &operator=(parameter &&lhs);
  parameter &operator=(const parameter_value_t &rhs);

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
inline CommittedValue<T> apply_change(const CommittedValue<T> &orig, const Change<T> &change);

template<>
inline CommittedValue<parameter> apply_change(const CommittedValue<parameter> &orig, const Change<parameter> &change){
  CommittedValue<parameter> result;
  result.CommitterId = change.CommitterId;
  if(change.Diff.Type == param_diff_type::set_value){
    result.Value = std::get<param_set_value_diff_data>(change.Diff.Data).Value;
  } else if (change.Diff.Type == param_diff_type::apply_op){
    auto &data = std::get<param_apply_op_diff_data>(change.Diff.Data);
    const numeric_value *param_val = std::get_if<numeric_value>(&orig.Value.value());
    if(param_val == nullptr){
      result.Value = orig.Value;
      return result;
    }

    //TODO : int/float shenanigans
    switch (data.OpType){
      case param_op_type::add:
        result.Value = *param_val + data.Args[0];
        break;
      case param_op_type::mul:
        result.Value = *param_val * data.Args[0];
        break;
      case param_op_type::mul_then_add:
        result.Value = *param_val * data.Args[0] + data.Args[1];
        break;
      default:
        result.Value = *param_val;
        break;
    }
  }
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
  //so far this is only used in current params construction, 
  //using an actual_changes obtained from compute_diff
  //compute_diff could only extract op information if CommittedValue contained it -- kinda hard to do.
  result.Diff.Type = param_diff_type::set_value;
  result.Diff.Data = param_set_value_diff_data{change.Change.second.Value.value()};
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
  std::map<entt::id_type, parameter_diff> ModifiedParams;
};

struct attributes_info_cumulative_changes {
  attributes_changes_t<status_t> StatusesChanges; 
  attributes_changes_t<parameter> ParamChanges;
};

template<typename T>
void obtain_priorities(const std::vector<Change<T>> &changes, const priority_callback_t &prio_func, std::vector<priority_t> &priorities){
  priorities.resize(changes.size(), 0);
  priority_request req;
  size_t counter = 0;
  for(const auto &edit : changes){
    req.EntitiesWithResultingPriorityValues.push_back(std::make_pair(edit.CommitterId, &priorities[counter]));
    counter++;
  }

  prio_func.Func(req, prio_func.UserData);
}

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
inline Change<T> merge_changes(const std::vector<Change<T>> &changes, const priority_callback_t &prio_func);

template<>
inline Change<status_t> merge_changes(const std::vector<Change<status_t>> &changes, const priority_callback_t &prio_func){
  std::vector<priority_t> priorities;
  obtain_priorities(changes, prio_func, priorities);

  Change<status_t> result;
  priority_t highest_modif_prio = std::numeric_limits<priority_t>::min();
  for(size_t i = 0; i < changes.size(); i++){
    priority_t prio_val = priorities[i];
    if(prio_val > highest_modif_prio){
      highest_modif_prio = prio_val;
      result = changes[i];
    }
  }

  return result;
}

template<>
inline Change<parameter> merge_changes(const std::vector<Change<parameter>> &changes, const priority_callback_t &prio_func){
  std::vector<priority_t> priorities;
  obtain_priorities(changes, prio_func, priorities);

  std::vector<size_t> setters;
  std::vector<size_t> operators;

  size_t i = 0; 
  for(const auto &change : changes){
    if(change.Diff.Type == param_diff_type::set_value){
      setters.push_back(i);
    } else if (change.Diff.Type == param_diff_type::apply_op) {
      operators.push_back(i);
    }
    i++;
  }

  Change<parameter> result;
  priority_t highest_setter_prio = std::numeric_limits<priority_t>::min();
  for(size_t s : setters){
    priority_t prio_val = priorities[s];
    if(prio_val > highest_setter_prio){
      highest_setter_prio = prio_val;
      result = changes[s];
    }
  }

  if(!setters.empty()){
    auto &starting_diff_data = std::get<param_set_value_diff_data>(result.Diff.Data);
    if(!std::holds_alternative<numeric_value>(starting_diff_data.Value))
    {
      return result;
    }
  }
  

  //Next up : apply operators incrementally!!!!
  //will need an operator fusion function.

  //this is for stacking Change operators together;
  //in pre-change triggers, I will probably need to modify operators with operations
  //(i.e. taking an addition value and multiplying it by a factor; (+ 2) x 5 becomes + 10)
  //here we have +2 stacked with x5 becomes (Value x 5 + 2).
  float total_add = 0.f;
  float total_mul = 1.f;

  for(size_t o : operators){
    auto data = std::get<param_apply_op_diff_data>(changes[o].Diff.Data);
    if(data.OpType == param_op_type::add){
      total_add += data.Args[0];
    } else if (data.OpType == param_op_type::mul){
      total_mul *= data.Args[0];
    } else if (data.OpType == param_op_type::mul_then_add){
      total_mul *= data.Args[0];
      total_add += data.Args[1];
    }
  }

  if(!setters.empty()){
    auto &starting_diff_data = std::get<param_set_value_diff_data>(result.Diff.Data);
    numeric_value &value = std::get<numeric_value>(starting_diff_data.Value);
    value = value * total_mul + total_add;
  } else if (!operators.empty()) {
    result.CommitterId = changes[operators[0]].CommitterId; //Hmm that's a weakness
    result.Diff.Type = param_diff_type::apply_op;
    param_apply_op_diff_data data;
    if (total_mul != 1.f && total_add == 0.f){
      data.OpType = param_op_type::mul;
      data.Args.resize(1, total_mul);
    } else if (total_mul != 1.f && total_add != 0.f){
      data.OpType = param_op_type::mul_then_add;
      data.Args.resize(2, 0.f);
      data.Args[0] = total_mul; data.Args[1] = total_add;
    } else {
      data.OpType = param_op_type::add;
      data.Args.resize(1, total_add);
    }
    result.Diff.Data = data;
  }
  
  return result;
}

template<typename T>
struct history_point {
  Change<T> ResultingChange;
  std::vector<Change<T>> SubmittedChanges;
};

template<typename T>
struct generic_history {
  std::map<timing_t, history_point<T>> History;
  void add_change(timing_t timing, const Change<T> &change);
  void merge_timing(timing_t timing, const priority_callback_t &prio_func);
  Change<T> cumulative_change(timing_t upper_bound, const priority_callback_t &prio_func) const;
};

template<typename T>
void generic_history<T>::add_change(timing_t timing, const Change<T> &change){
  auto &&hist_point = History.emplace(timing, history_point<T>()).first->second;
  hist_point.SubmittedChanges.push_back(change);
}

template<typename T>
void generic_history<T>::merge_timing(timing_t timing, const priority_callback_t &prio_func){
  auto it = History.find(timing);
  if(it == History.end()){
    return;
  }
  auto &hist_point = it->second;
  hist_point.ResultingChange = merge_changes(hist_point.SubmittedChanges, prio_func);
}

template<typename T>
Change<T> generic_history<T>::cumulative_change(timing_t upper_bound, const priority_callback_t &prio_func) const {
  Change<T> result;
  result.CommitterId = entt::null;
  diff_merger<T> merger;

  auto it = History.begin();
  while(it != History.end() && it->first <= upper_bound){
    //merge because changes may become incremental
    result = merger.merge_changes(result, it->second.ResultingChange, prio_func);
    it++;
  }
  
  return result;
}

struct attributes_info_history {
  const attributes_info_snapshot StartingPoint;
  
  std::map<entt::id_type, generic_history<status_t>> StatusesHistory; //DA FUTURE
  std::map<entt::id_type, generic_history<parameter>> ParamsHistory;

  logistics_API bool add_changes(timing_t timing, const attributes_info_cumulative_changes &changes);
  logistics_API void merge_timing(timing_t timing, const priority_callback_t &callback);

  logistics_API attributes_info_snapshot produce_snapshot(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  logistics_API attributes_info_cumulative_changes cumulative_changes(timing_t upper_bound = std::numeric_limits<timing_t>::max()) const;
  
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