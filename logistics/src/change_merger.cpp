#include "change_merger.h"
#include "attributes_info.h"

namespace logistics {

  merge_result simple_change_merger::merge_changes(const attributes_info_changes &left, const attributes_info_changes &right, attributes_info_changes &result){
    merge_result ret = merge_result::success;
    result = left;
    for(const auto &[hash, status_mod] : right.ModifiedStatuses){
      if(result.ModifiedStatuses.find(hash) == result.ModifiedStatuses.end()){
        result.ModifiedStatuses.emplace(hash, status_mod);
      } else if(status_mod != result.ModifiedStatuses.at(hash)) {
        ret = merge_result::conflict;
      }
    }

    for(const auto &[hash, param_pair] : right.ModifiedParams){
      if(result.ModifiedParams.find(hash) == result.ModifiedParams.end()){
        result.ModifiedParams.emplace(hash, param_pair);
      } else {
        auto &left_param_pair = result.ModifiedParams.at(hash);
        if(left_param_pair.first.value() != param_pair.first.value() 
          || left_param_pair.second.value() != param_pair.second.value()){
          ret = merge_result::conflict;
        }
      }
    }

    return ret;
  }

}