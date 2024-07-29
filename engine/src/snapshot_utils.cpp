#include "snapshot_utils.h"

bool get_value_for_status(const attributes_info_snapshot &snapshot, entt::id_type hash){
  auto it = snapshot.StatusHashes.Values.find(hash);
  return it != snapshot.StatusHashes.Values.end() && it->second.Value == true;
}

parameter get_value_for_parameter(const attributes_info_snapshot &snapshot, entt::id_type hash){
  auto it = snapshot.ParamValues.Values.find(hash);
  if(it == snapshot.ParamValues.Values.end()){
    return parameter{};
  } else {
    return it->second.Value;
  }
}