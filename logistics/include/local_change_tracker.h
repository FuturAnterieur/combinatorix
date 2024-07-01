#pragma once

#include "attributes_info.h"
#include <vector>

namespace logistics{

  template<typename ValueType>
  struct change_tracker_edits_for_hash {
    std::vector<ValueType> Values;
    void apply_to_value(ValueType &value){
      value = Values.back();
    }
  };

  template<typename ValueType>
  struct change_tracker_category{
    std::map<entt::id_type, change_tracker_edits_for_hash<ValueType>> Edits;

    void edit(entt::id_type hash, const ValueType &value){
      auto &&tracker = Edits.emplace(hash, change_tracker_edits_for_hash<ValueType>()).first->second;
      tracker.Values.push_back(value);
    }

    bool get_active_value(entt::id_type hash, ValueType &result){
      auto it = Edits.find(hash);
      if(it == Edits.end()){
        return false;
      } 

      result = it->second.Values.back();

      return true;
    }
  };

  class local_change_tracker {
    public:
      local_change_tracker() =default;
      ~local_change_tracker() =default;

      void set_starting_point(const attributes_info_snapshot &snapshot);

      void apply_status_edit(entt::id_type status_hash, bool value);
      void apply_param_edit(entt::id_type param_hash, const parameter &value);

      bool get_active_value_for_status(entt::id_type status_hash);
      parameter get_active_value_for_parameter(entt::id_type param_hash);

      attributes_info_snapshot produce_active_snapshot();

    private:
      attributes_info_snapshot StartingPoint; //will be the last IntrinsicValues, not the ones previously commited to the active branch
      //attributes_info_snapshot FinalComparisonReference; //will be the last commited

      change_tracker_category<bool> StatusEdits;
      change_tracker_category<parameter> ParameterEdits;
  };
}
