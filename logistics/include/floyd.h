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
    
    
    size_t tortoise = x0;
    size_t hare = x0 + 1;
    while (hare < size && tortoise < size){
      if(vec[hare] == vec[tortoise]){
        would_be_cycle = true;
        break;
      }

      tortoise += 1;
      hare += 2;
    }
    
    if(!would_be_cycle){
      return false;
    }

    size_t would_be_cycle_len = hare - tortoise;
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
