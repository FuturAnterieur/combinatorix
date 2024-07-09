#pragma once

#include <vector>
namespace floyd {
  
  template<typename ElementType>
  bool find_cycle(const std::vector<ElementType> &vec, size_t x0, size_t &start, size_t &end){
    size_t size = vec.size();
    bool would_be_cycle = false;
    if(size <= 2){
      return false;
    }
    
    
    int64_t tortoise = static_cast<int64_t>(size) - 1;
    int64_t hare = static_cast<int64_t>(size) - 2;
    while (hare >= 0 && tortoise >= 0){
      if(vec[hare] == vec[tortoise]){
        would_be_cycle = true;
        break;
      }

      tortoise -= 1;
      hare -= 2;
    }
    
    if(!would_be_cycle){
      return false;
    }

    size_t would_be_cycle_len = tortoise - hare;
    size_t mu = 0;
    while(mu < size && mu + would_be_cycle_len < size){
      if(vec[mu] == vec[mu + would_be_cycle_len]) {
        break;
      }
      mu++;
    }

    if(mu >= size){
      return false;
    }

    size_t i = 0;
    while(i < would_be_cycle_len && mu + i < size){
      if(vec[mu + i] != vec[mu + would_be_cycle_len + i]){
        return false;
      }
      i++;
    }

    start = mu;
    end = mu + would_be_cycle_len;

    return true;
  }

}
