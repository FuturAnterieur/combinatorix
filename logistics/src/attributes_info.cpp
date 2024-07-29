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
attributes_info_cumulative_changes cumul_changes_from_long(const attributes_info_changes &changes){
  attributes_info_cumulative_changes result;
  
  //result.StatusesChanges = shorten_changes(changes.ModifiedStatuses);
  for(const auto &[hash, change] : changes.ModifiedStatuses){
    result.StatusesChanges.emplace(hash, shorten_change(change));
  }

  for(const auto &[hash, param_pair] : changes.ModifiedParams){
    result.ParamChanges.emplace(hash, shorten_change(param_pair));
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
  return changes;
}

//=====================================
bool paste_cumulative_changes(const attributes_info_cumulative_changes &changes, attributes_info_snapshot &snapshot)
{
  //apply_changes_to_snapshot<status_t>(changes.StatusesChanges, snapshot.StatusHashes);

  for(const auto &[hash, change] : changes.ParamChanges){
    CommittedValue<parameter> existing;
    auto it = snapshot.ParamValues.Values.find(hash);
    if(it != snapshot.ParamValues.Values.end()){
      existing = it->second;
    }
    snapshot.ParamValues.Values.insert_or_assign(hash, apply_change(existing, change));
  }
  for(const auto &[hash, change] : changes.StatusesChanges){
    CommittedValue<status_t> existing;
    auto it = snapshot.StatusHashes.Values.find(hash);
    if(it != snapshot.StatusHashes.Values.end()){
      existing = it->second;
    }
    snapshot.StatusHashes.Values.insert_or_assign(hash, apply_change(existing, change));
  }
  return true;
}

//=====================================
bool attributes_info_history::add_changes(timing_t timing, const attributes_info_cumulative_changes &changes, const priority_callback_t &callback){
  
  for(const auto &[hash, change] : changes.ParamChanges){
    auto &hist = ParamsHistory.emplace(hash, generic_history<parameter>()).first->second;
    hist.add_change(timing, change, callback);
  }
    
  for(const auto &[hash, change] : changes.StatusesChanges){
    auto &hist = StatusesHistory.emplace(hash, generic_history<status_t>()).first->second;
    hist.add_change(timing, change, callback);
  }

  return true;
}

//=====================================
attributes_info_snapshot attributes_info_history::produce_snapshot(timing_t upper_bound) const{
  attributes_info_snapshot snapshot = StartingPoint;
  
  //calculate priority here too!
  attributes_info_cumulative_changes cumul = cumulative_changes(upper_bound);
  paste_cumulative_changes(cumul, snapshot);
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

  for(const auto &[hash, hist] : ParamsHistory){
    cumul.ParamChanges.emplace(hash, hist.cumulative_change(upper_bound, timing_only_callback));
  }
    
  for(const auto &[hash, hist] : StatusesHistory){
    cumul.StatusesChanges.emplace(hash, hist.cumulative_change(upper_bound, timing_only_callback));
  }
  return cumul;
}

//=====================================
attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot){
  attributes_info_changes changes;

  attributes_detailed_changes_t<status_t> statuses = compute_diff(old_snapshot.StatusHashes, new_snapshot.StatusHashes);
  attributes_detailed_changes_t<parameter> params = compute_diff(old_snapshot.ParamValues, new_snapshot.ParamValues);

  changes.ModifiedParams = params.Changes;
  changes.ModifiedStatuses = statuses.Changes;

  /*
  const CommittedValue<parameter> param_null = CommittedValue<parameter>{parameter{}, entt::null};
  for(const auto &[hash, val] : new_snapshot.StatusHashes.Values){
    auto old_it = old_snapshot.StatusHashes.Values.find(hash);
    if(old_it == old_snapshot.StatusHashes.Values.end() || (old_it->second.Value == false && val.Value == true))
    {
      changes.ModifiedStatuses.emplace(hash, Change<status_t>{smt::added, val.CommitterId});
    }
  }

  for(const auto &[hash, val] : old_snapshot.Values.StatusHashes){
    auto new_it = new_snapshot.StatusHashes.find(hash);
    entt::entity CommitterId = entt::null;
    if(new_it == new_snapshot.StatusHashes.end() || (new_it->second.Value == false && val.Value == true)){
      if(new_it != new_snapshot.StatusHashes.end()){
        CommitterId = new_it->second.CommitterId;
      }
      changes.ModifiedStatuses.emplace(hash, Change<status_t>{smt::removed, CommitterId});
    }
  }

  for(const auto &[hash, param] : new_snapshot.ParamValues){
    if(old_snapshot.ParamValues.find(hash) == old_snapshot.ParamValues.end()){
      changes.ModifiedParams.emplace(hash, std::make_pair(param_null, param));
    } else {
      auto previous_value = old_snapshot.ParamValues.at(hash);
      if(previous_value.Value.value() != param.Value.value()){
        changes.ModifiedParams.emplace(hash, std::make_pair(previous_value, param));
      }
    }
  }

  for(const auto &[hash, param] : old_snapshot.ParamValues){
    if(new_snapshot.ParamValues.find(hash) == new_snapshot.ParamValues.end()){
      changes.ModifiedParams.emplace(hash, std::make_pair(param, param_null));
    } 
  }*/

  return changes;
}
