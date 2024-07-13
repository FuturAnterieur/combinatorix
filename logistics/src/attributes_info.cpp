#include "change_merger.h"
#include "attributes_info.h"
#include "entt_utils.h"

struct parameter::pimpl{
  parameter_value_t Value;
};

parameter::parameter() : _Pimpl(new pimpl()) {
  DT = data_type::null;
  _Pimpl->Value = "";
}

parameter::parameter(const std::string &val) :  _Pimpl(new pimpl()){
  DT = concrete_to_enum_type<std::string>::t;
  _Pimpl->Value = val;
}

parameter::parameter(const char *val) : _Pimpl(new pimpl()) {
  DT = concrete_to_enum_type<std::string>::t;
  _Pimpl->Value = std::string(val);
}

parameter::parameter(float val) : _Pimpl(new pimpl()){
  DT = data_type::number;
  _Pimpl->Value = val;
}

parameter::parameter(int64_t val) :  _Pimpl(new pimpl()){
  DT = data_type::integer;
  _Pimpl->Value = val;
}

parameter::parameter(bool val) : _Pimpl(new pimpl()){
  DT = concrete_to_enum_type<bool>::t;
  _Pimpl->Value = val;
}

parameter::parameter(const parameter &other) : _Pimpl(new pimpl()){
  DT = other.DT;
  _Pimpl->Value = other._Pimpl->Value;
}

parameter::parameter(parameter &&other) : _Pimpl(other._Pimpl) {
  other._Pimpl = nullptr;
  DT = other.DT;
}

parameter::~parameter(){
  delete _Pimpl;
}

parameter &parameter::operator=(const parameter &lhs){
  _Pimpl->Value = lhs.value();
  DT = lhs.DT;
  return *this;
}

parameter &parameter::operator=(parameter &&lhs){
  if(_Pimpl){
    delete _Pimpl;
  }
  _Pimpl = lhs._Pimpl;
  lhs._Pimpl = nullptr;
  DT = lhs.DT;

  return *this;
}

data_type parameter::dt() const {
  return DT;
}

const parameter_value_t &parameter::value() const {
  return _Pimpl->Value;
}

parameter_value_t &parameter::access_value(){
  return _Pimpl->Value;
}

void parameter::set_value(const parameter_value_t value) {
  _Pimpl->Value = value;
}

data_type &parameter::access_data_type(){
  return DT;
}

//=================================================
attributes_info_short_changes short_changes_from_changes(const attributes_info_changes &changes){
  attributes_info_short_changes result;
  result.ModifiedStatuses = changes.ModifiedStatuses;
  for(const auto &[hash, param_pair] : changes.ModifiedParams){
    result.ModifiedParams.emplace(hash, param_pair.second);
  }
  return result;
}


//=====================================
bool paste_attributes_changes(const attributes_info_short_changes &changes, attributes_info_reference &ref)
{
  for(const auto &[hash, param] : changes.ModifiedParams){
    
    if(param.dt() == data_type::null){ //deletion
      ref.ParamValues.erase(hash);
      //assert(ret);
    } else  { //modification
      ref.ParamValues.insert_or_assign(hash, param);
    }
  }
  for(const auto &[hash, smt_val] : changes.ModifiedStatuses){
    
    if(smt_val == smt::removed){
      ref.StatusHashes.erase(hash);
      //assert(ret);
    } else {
      ref.StatusHashes.insert(hash);
    }
  }
  return true;
}

//=====================================
bool attributes_info_history::add_changes(entt::registry &registry, timing_t timing, const attributes_info_state_at_timing &changes){
  using namespace logistics;
  auto &state = History.emplace(timing, attributes_info_state_at_timing{}).first->second;
  
  attributes_info_short_changes copy = state.Changes;
  attributes_info_changes_comparable left{copy, timing};
  attributes_info_changes_comparable right{changes.Changes, timing};

  calculate_priority(registry, state.OriginatingEntity, changes.OriginatingEntity, left.Priority, right.Priority);

  simple_change_merger merger;
  auto result = merger.merge_changes(left, right, state.Changes);
  if(result == merge_result::conflict){
    return false;
  }
  
  return true;
}

//=====================================
attributes_info_snapshot attributes_info_history::produce_snapshot(timing_t upper_bound) const{
  attributes_info_snapshot snapshot = StartingPoint;
  attributes_info_short_changes changes;
  attributes_info_reference ref(snapshot);
  //calculate priority here too!
  cumulative_changes(changes, upper_bound);
  paste_attributes_changes(changes, ref);
  return snapshot;
}

//=====================================
bool attributes_info_history::cumulative_changes(attributes_info_short_changes &changes, timing_t upper_bound) const{
  using namespace logistics;
  simple_change_merger merger;
  return cumulative_changes_swiss_knife(changes, 0, upper_bound, &merger);
}

//=====================================
bool attributes_info_history::cumulative_changes_disregarding_timing(attributes_info_short_changes &changes, timing_t lower_bound, timing_t upper_bound) const{
  using namespace logistics;
  timing_less_merger merger;
  return cumulative_changes_swiss_knife(changes, lower_bound, upper_bound, &merger);
}

//=====================================
bool attributes_info_history::cumulative_changes_swiss_knife(attributes_info_short_changes &changes, timing_t lower_bound, timing_t upper_bound, logistics::change_merger *merger) const {
  using namespace logistics;
  attributes_info_changes temp;
  merge_result result = merge_result::success;


  auto it = History.begin();
  while(it != History.end() && it->first < lower_bound){
    it++;
  }

  timing_t time_tracker = lower_bound;
  while(it != History.end() && it->first < upper_bound){
    attributes_info_short_changes copy = changes;
    attributes_info_changes_comparable left{copy, time_tracker, 0};
    attributes_info_changes_comparable right{it->second.Changes, it->first, 0};
    result = merger->merge_changes(left, right, changes);
    if(result == merge_result::conflict){
      return false;
    }
    time_tracker = it->first;
    it++;
  }
  return true;

}


//=====================================
attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot){
  attributes_info_changes changes;

  const parameter param_null = parameter();
  for(const auto &hash : new_snapshot.StatusHashes){
    if(old_snapshot.StatusHashes.find(hash) == old_snapshot.StatusHashes.end()){
      changes.ModifiedStatuses.emplace(hash, smt::added);
    }
  }

  for(const auto &hash : old_snapshot.StatusHashes){
    if(new_snapshot.StatusHashes.find(hash) == new_snapshot.StatusHashes.end()){
      changes.ModifiedStatuses.emplace(hash, smt::removed);
    }
  }

  for(const auto &[hash, param] : new_snapshot.ParamValues){
    if(old_snapshot.ParamValues.find(hash) == old_snapshot.ParamValues.end()){
      changes.ModifiedParams.emplace(hash, std::make_pair(param_null, param));
    } else {
      parameter previous_value = old_snapshot.ParamValues.at(hash);
      if(previous_value.value() != param.value()){
        changes.ModifiedParams.emplace(hash, std::make_pair(previous_value, param));
      }
    }
  }

  for(const auto &[hash, param] : old_snapshot.ParamValues){
    if(new_snapshot.ParamValues.find(hash) == new_snapshot.ParamValues.end()){
      changes.ModifiedParams.emplace(hash, std::make_pair(param, param_null));
    } 
  }

  return changes;
}
