#include "change_merger.h"
#include "attributes_info.h"

namespace logistics {

  merge_result simple_change_merger::merge_changes(attributes_info_changes left, const attributes_info_changes &right, attributes_info_changes &result){
    assert(left.Timing <= right.Timing);
    
    merge_result ret = merge_result::success;
    result = left;
    for(const auto &[hash, status_mod] : right.ModifiedStatuses){
      if(result.ModifiedStatuses.find(hash) == result.ModifiedStatuses.end() || right.Timing > left.Timing || right.Priority > left.Priority){
        result.ModifiedStatuses.insert_or_assign(hash, status_mod);
      } else if(left.Priority == right.Priority && left.Timing == right.Timing && status_mod != result.ModifiedStatuses.at(hash)) {
        ret = merge_result::conflict;
      }
    }

    for(const auto &[hash, param_pair] : right.ModifiedParams){
      bool left_does_not_have_hash = result.ModifiedParams.find(hash) == result.ModifiedParams.end();
      if(left_does_not_have_hash || right.Timing > left.Timing || right.Priority > left.Priority){
        if(left_does_not_have_hash || right.Timing == left.Timing){
          result.ModifiedParams.insert_or_assign(hash, param_pair);
        } else {
          auto &pair_existing_in_result = result.ModifiedParams.at(hash);
          pair_existing_in_result.second = param_pair.second; //concatenate if existing
        }
      } else if(left.Timing == right.Timing && left.Priority == right.Priority) {
        auto &left_param_pair = result.ModifiedParams.at(hash);
        if(left_param_pair.first.value() != param_pair.first.value() 
          || left_param_pair.second.value() != param_pair.second.value()){
          ret = merge_result::conflict;
        }
      }
    }

    if(ret == merge_result::conflict){
      result = left;
    }
    return ret;
  }

}