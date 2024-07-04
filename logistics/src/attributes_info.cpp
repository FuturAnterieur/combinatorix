#include "attributes_info.h"

attributes_info_changes compute_diff(const attributes_info_snapshot &old_snapshot, const attributes_info_snapshot &new_snapshot){
  attributes_info_changes changes;

  const parameter param_null = parameter{data_type::null, ""};
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
      if(previous_value.Value != param.Value){
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