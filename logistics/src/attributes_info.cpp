#include "attributes_info.h"
#include "entt_utils.h"

struct parameter::pimpl{
  parameter_value_t Value;
};

parameter::parameter() : _Pimpl(new pimpl()) {
  DT = data_type::null;
  _Pimpl->Value = "";
}

parameter::parameter(const parameter_value_t &val) : _Pimpl(new pimpl()) {
  DT = data_type::not_null;
  _Pimpl->Value = val;
}

parameter::parameter(const std::string &val) :  _Pimpl(new pimpl()){
  DT = concrete_to_enum_type<std::string>::t;
  _Pimpl->Value = val;
}

parameter::parameter(const char *val) : _Pimpl(new pimpl()) {
  DT = concrete_to_enum_type<std::string>::t;
  _Pimpl->Value = std::string(val);
}

parameter::parameter(const numeric_value &val) : _Pimpl(new pimpl()){
  DT = data_type::number;
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

parameter &parameter::operator=(const parameter_value_t &rhs)
{
  _Pimpl->Value = rhs;
  //TODO : ditch DT I think, in favor of a simple "not null" indicator
  DT = data_type::not_null;
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
  
  result.StatusesChanges = shorten_changes(changes.ModifiedStatuses);
  result.ParamChanges = shorten_changes(changes.ModifiedParams);
  return result;
}

//=========================================
bool changes_empty(attributes_info_changes &changes){
  return changes.ModifiedStatuses.Changes.empty() && changes.ModifiedParams.Changes.empty();
}

parameter_diff diff_from_set_val(parameter_value_t val){
  parameter_diff result;
  result.Type = param_diff_type::set_value;
  result.Data = param_set_value_diff_data{val, true};
  return result;
}

parameter_diff diff_from_op(param_op_type op, float arg)
{
  parameter_diff result;
  result.Type = param_diff_type::apply_op;
  result.Data = param_apply_op_diff_data{op, {arg}};
  return result;
}

attributes_info_cumulative_changes cumul_changes_from_short(const attributes_info_short_changes &short_changes, entt::entity originating_entity)
{
  attributes_info_cumulative_changes changes;
  populate_change_map<status_t>(changes.StatusesChanges.Changes, short_changes.ModifiedStatuses, originating_entity);
  populate_change_map<parameter>(changes.ParamChanges.Changes, short_changes.ModifiedParams, originating_entity);
  return changes;
}

//=====================================
bool paste_cumulative_changes(const attributes_info_cumulative_changes &changes, attributes_info_snapshot &snapshot)
{
  apply_changes_to_snapshot<status_t>(changes.StatusesChanges, snapshot.StatusHashes);
  apply_changes_to_snapshot<parameter>(changes.ParamChanges, snapshot.ParamValues);
  
  return true;
}

//=====================================
bool attributes_info_history::add_changes(timing_t timing, const attributes_info_cumulative_changes &changes){
  
  for(const auto &[hash, change] : changes.ParamChanges.Changes){
    auto &hist = ParamsHistory.emplace(hash, generic_history<parameter>()).first->second;
    hist.add_change(timing, change);
  }
    
  for(const auto &[hash, change] : changes.StatusesChanges.Changes){
    auto &hist = StatusesHistory.emplace(hash, generic_history<status_t>()).first->second;
    hist.add_change(timing, change);
  }

  return true;
}

void attributes_info_history::merge_timing(timing_t timing, const priority_callback_t &callback)
{
  for(auto &[hash, hist] : StatusesHistory){
    hist.merge_timing(timing, callback);
  }

  for(auto &[hash, hist] : ParamsHistory){
    hist.merge_timing(timing, callback);
  }
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
    cumul.ParamChanges.Changes.emplace(hash, hist.cumulative_change(upper_bound, timing_only_callback));
  }
    
  for(const auto &[hash, hist] : StatusesHistory){
    cumul.StatusesChanges.Changes.emplace(hash, hist.cumulative_change(upper_bound, timing_only_callback));
  }
  return cumul;
}

//=====================================
attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot){
  attributes_info_changes changes;

  changes.ModifiedStatuses = compute_diff(old_snapshot.StatusHashes, new_snapshot.StatusHashes);
  changes.ModifiedParams = compute_diff(old_snapshot.ParamValues, new_snapshot.ParamValues);

  return changes;
}
