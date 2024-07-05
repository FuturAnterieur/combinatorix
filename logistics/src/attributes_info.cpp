#include "attributes_info.h"


struct parameter::pimpl{
  parameter_value_t Value;
};

parameter::parameter() : _Pimpl(new pimpl()) {
  DT = data_type::null;
  _Pimpl->Value = "";
}

parameter::parameter(const std::string &val) :  _Pimpl(new pimpl()){
  DT = data_type::string;
  _Pimpl->Value = val;
}

parameter::parameter(const char *val) : _Pimpl(new pimpl()) {
  DT = data_type::string;
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
  DT = data_type::boolean;
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

attributes_info_changes compute_changes_diff(const attributes_info_changes &old_changes, const attributes_info_changes &new_changes){
  const parameter param_null = parameter{};
  attributes_info_changes changes;
  for(const auto &[hash, mod] : new_changes.ModifiedStatuses){
    if(old_changes.ModifiedStatuses.find(hash) == old_changes.ModifiedStatuses.end()){
      changes.ModifiedStatuses.emplace(hash, mod);
    }
  }

  for(const auto &[hash, mod] : new_changes.ModifiedParams){
    if(old_changes.ModifiedParams.find(hash) == old_changes.ModifiedParams.end()){
      changes.ModifiedParams.emplace(hash, mod);
    }
  }
  return changes;
}