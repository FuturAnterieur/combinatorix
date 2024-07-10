#pragma once 

#include "attributes_info.h"

struct attributes_info_changes_comparable {
  const attributes_info_changes &Changes;
  timing_t Timing{0}; 
  priority_t Priority{0}; //Used only sparingly for now. This is not the final priority mechanic. The real one should be based on customizable game rules.
};

namespace logistics {
  enum class merge_result {
    success,
    conflict
  };

  class change_merger {
    public:
      virtual merge_result merge_changes(const attributes_info_changes_comparable &left, const attributes_info_changes_comparable &right, attributes_info_changes &result) const = 0;
  };

  class simple_change_merger : public change_merger {
    public:
      virtual merge_result merge_changes(const attributes_info_changes_comparable & left, const attributes_info_changes_comparable &right, attributes_info_changes &result) const;
  };

  class timing_less_merger : public change_merger {
    public:
      virtual merge_result merge_changes(const attributes_info_changes_comparable & left, const attributes_info_changes_comparable &right, attributes_info_changes &result) const;
  };
}


