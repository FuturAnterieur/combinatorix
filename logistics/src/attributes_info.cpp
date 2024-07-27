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

//=========================================
bool changes_empty(attributes_info_changes &changes){
  return changes.ModifiedStatuses.empty() && changes.ModifiedParams.empty();
}

attributes_info_cumulative_changes cumul_changes_from_short(const attributes_info_short_changes &short_changes, entt::entity originating_entity){
  attributes_info_cumulative_changes changes;
  populate_change_map<status_t>(changes.StatusesChanges, short_changes.ModifiedStatuses, originating_entity);
  populate_change_map<parameter>(changes.ParamChanges, short_changes.ModifiedParams, originating_entity);
}

//=====================================
bool paste_cumulative_changes(const attributes_info_cumulative_changes &changes, attributes_info_snapshot &snapshot)
{
  for(const auto &[hash, change] : changes.ParamChanges){
    
    if(change.Diff.dt() == data_type::null){ //deletion
      snapshot.ParamValues.erase(hash);
    } else  { //modification
      snapshot.ParamValues.insert_or_assign(hash, CommittedValue<parameter>{change.Diff, change.CommitterId});
    }
  }
  for(const auto &[hash, change] : changes.StatusesChanges){
    
    if(change.Diff == smt::removed){
      snapshot.StatusHashes.erase(hash);
    } else {
      snapshot.StatusHashes.insert_or_assign(hash, CommittedValue<status_t>{true, change.CommitterId});
    }
  }
  return true;
}

//=====================================
bool attributes_info_history::add_changes(timing_t timing, const attributes_info_short_changes &changes, entt::entity originating_entity, const priority_callback_t &callback, void *cb_user_data){
  using namespace logistics;

  for(const auto &[hash, param] : changes.ModifiedParams){
    auto &hist = ParamsHistory2.emplace(hash, generic_history<parameter>()).first->second;
    hist.add_change(timing, param, callback);
  }
    
  for(const auto &[hash, smt_val] : changes.ModifiedStatuses){
    auto &hist = StatusesHistory2.emplace(hash, generic_history<status_t>()).first->second;
    hist.add_change(timing, smt_val, callback);
  }

  /*
  auto &state = History.emplace(timing, attributes_info_state_at_timing{}).first->second;
  
  attributes_info_short_changes copy = state.Changes;
  attributes_info_changes_comparable left{copy, timing};
  attributes_info_changes_comparable right{changes.Changes, timing};

  priority_request req{{std::make_pair(state.OriginatingEntity, &left.Priority), std::make_pair(changes.OriginatingEntity, &right.Priority)}};
  callback(req, cb_user_data);
  
  simple_change_merger merger;
  auto result = merger.merge_changes(left, right, state.Changes);
  if(result == merge_result::conflict){
    return false;
  }*/
  
  return true;
}

//=====================================
attributes_info_snapshot attributes_info_history::produce_snapshot(timing_t upper_bound) const{
  attributes_info_snapshot snapshot = StartingPoint;
  attributes_info_short_changes changes;
  attributes_info_reference ref(snapshot);
  //calculate priority here too!
  cumulative_changes(changes, upper_bound);
  paste_cumulative_changes(changes, ref);
  return snapshot;
}

//=====================================
attributes_info_cumulative_changes attributes_info_history::cumulative_changes(timing_t upper_bound) const {
  attributes_info_cumulative_changes cumul;
  priority_callback timing_only_callback;
  timing_only_callback.Func = [](priority_request &req, void *){
    for(size_t i = 0; i < req.EntitiesWithResultingPriorityValues.size(); i++){
      auto &pair = req.EntitiesWithResultingPriorityValues[i];
      *(pair.second) = i; //just assume they will be sorted by ascending timing
    }
  };
  timing_only_callback.UserData = nullptr;

  for(const auto &[hash, hist] : ParamsHistory2){
    cumul.ParamChanges.emplace(hash, hist.cumulative_change(upper_bound, timing_only_callback));
  }
    
  for(const auto &[hash, hist] : StatusesHistory2){
    cumul.StatusesChanges.emplace(hash, hist.cumulative_change(upper_bound, timing_only_callback));
  }
}

/*
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
  while(it != History.end() && it->first <= upper_bound){
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
*/

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
