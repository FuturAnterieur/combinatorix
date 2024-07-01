#include "local_change_tracker.h"


namespace logistics{
  
  void local_change_tracker::set_starting_point(const attributes_info_snapshot &snapshot){
    StartingPoint = snapshot;
  }

  void local_change_tracker::apply_status_edit(entt::id_type status_hash, bool value){
    StatusEdits.edit(status_hash, value);
  }

  void local_change_tracker::apply_param_edit(entt::id_type param_hash, const parameter &value){
    ParameterEdits.edit(param_hash, value);
  }

  bool local_change_tracker::get_active_value_for_status(entt::id_type status_hash){
    bool result;
    if(!StatusEdits.get_active_value(status_hash, result)){
      result = StartingPoint.StatusHashes.find(status_hash) != StartingPoint.StatusHashes.end();
    }
    return result;
  }

  parameter local_change_tracker::get_active_value_for_parameter(entt::id_type param_hash){
    parameter result;
    if(!ParameterEdits.get_active_value(param_hash, result)){
      result = StartingPoint.ParamValues.at(param_hash);
    }
    return result;
  }
};