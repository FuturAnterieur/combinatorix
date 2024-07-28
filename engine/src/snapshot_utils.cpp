#include "snapshot_utils.h"

bool get_value_for_status(const attributes_info_snapshot &snapshot, entt::id_type hash){
  auto it = snapshot.StatusHashes.find(hash);
  return it != snapshot.StatusHashes.end() && it->second.Value == true;
}

parameter get_value_for_parameter(const attributes_info_snapshot &snapshot, entt::id_type hash){
  auto it = snapshot.ParamValues.find(hash);
  if(it == snapshot.ParamValues.end()){
    return parameter{};
  } else {
    return it->second.Value;
  }
}