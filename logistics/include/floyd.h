#pragma once

#include <vector>
namespace floyd {
  
  template<typename ElementType>
  bool find_cycle(const std::vector<ElementType> &vec, size_t x0, size_t &start, size_t &end){
    size_t size = vec.size();
    bool ret = false;
    if(size <= 2){
      return ret;
    }
    
    
    size_t tortoise = x0;
    size_t hare = x0 + 1;
    while (hare < size && tortoise < size){
      if(vec[hare] == vec[tortoise]){
        return true;
      }

      tortoise += 1;
      hare += 2;
    }
    //that's part one!!!!
    return false;
  }

}
