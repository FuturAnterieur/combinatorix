#pragma once 


struct attributes_info_changes;
namespace logistics {
  enum class merge_result {
    success,
    conflict
  };

  class change_merger {
    public:
      virtual merge_result merge_changes(attributes_info_changes left, const attributes_info_changes &right, attributes_info_changes &result) = 0;
  };

  class simple_change_merger : public change_merger {
    public:
      virtual merge_result merge_changes(attributes_info_changes left, const attributes_info_changes &right, attributes_info_changes &result);
  };
}

