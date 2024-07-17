#pragma once

#include "attributes_info.h"
#include "history_utils.h"

namespace logistics{
  template<changes_category category>
  struct history_storage {
    private:
      entt::id_type ActiveBranchHash;
      entt::registry *_Registry;

    public:
    history_storage() = default;
    ~history_storage() = default;

    inline void set_registry(entt::registry *registry){
      _Registry = registry;
    }

    inline void set_branch_name(const std::string &name){
      std::string long_string = name + std::to_string(category) + std::string(entt::type_name<attributes_info_history>().value());
      ActiveBranchHash = entt::hashed_string::value(long_string.data());
    }

    inline void init_starting_point(entt::entity entity){
      auto &storage = get_changes_storage();
      if(!storage.contains(entity)){
        attributes_info_snapshot starting_point;
        auto &stable_info = _Registry->get<attributes_info>(entity);
        
        if constexpr (category == changes_category::intrinsics) {
          starting_point.StatusHashes = stable_info.IntrinsicStatusHashes;
          starting_point.ParamValues = stable_info.IntrinsicParamValues;
        } else if constexpr (category == changes_category::current){
          starting_point.StatusHashes = stable_info.CurrentStatusHashes;
          starting_point.ParamValues = stable_info.CurrentParamValues;
        }

        storage.emplace(entity, starting_point);
      }
    }

    inline void init_starting_point(entt::entity entity, const attributes_info_snapshot &snapshot){
      auto &storage = get_changes_storage();
      if(!storage.contains(entity)){
        storage.emplace(entity, snapshot);
      }
    }
    
    inline void commit_changes(entt::entity entity, const attributes_info_changes &changes, entt::entity originating_entity, timing_t timing){
      auto &status_changes_storage = get_changes_storage();
      init_starting_point(entity);

      auto &history = status_changes_storage.get(entity);

      attributes_info_state_at_timing state;
      state.Changes = short_changes_from_changes(changes);
      state.OriginatingEntity = originating_entity;
      
      history.add_changes(timing, state, classic_priority_callback, _Registry);
    }

    inline void merge_to_reality(timing_t upper_bound){
      auto &changes_storage = get_changes_storage();
      auto status_changes_view = entt::view<entt::get_t<attributes_info_history>>{changes_storage};

      for(entt::entity entity : status_changes_view){
        assert(_Registry->all_of<attributes_info>(entity));
        auto &attr_info = _Registry->get<attributes_info>(entity);

        auto &history = status_changes_view.get<attributes_info_history>(entity);

        attributes_info_short_changes cumulative_changes;
        history.cumulative_changes(cumulative_changes, upper_bound);

        if constexpr (category == changes_category::current){
          attributes_info_reference ref{attr_info.CurrentStatusHashes, attr_info.CurrentParamValues};
          paste_attributes_changes(*_Registry, entity, cumulative_changes, ref, true, true);
        } else if constexpr (category == changes_category::intrinsics){
          attributes_info_reference ref{attr_info.IntrinsicStatusHashes, attr_info.IntrinsicParamValues};
          paste_attributes_changes(*_Registry, entity, cumulative_changes, ref, false, true);
        }
      }
      changes_storage.clear();
    }

    inline status_changes_storage_t &get_changes_storage(){
      return _Registry->storage<attributes_info_history>(ActiveBranchHash);
    }

    inline attributes_info_snapshot get_most_recent_snapshot(entt::entity entity) {
      auto &storage = get_changes_storage();
      if(storage.contains(entity)){
        
        auto &history = storage.get(entity);
        return history.produce_snapshot(); 
      }

      attributes_info_snapshot stable_values;
      auto &info = _Registry->get<attributes_info>(entity);
      if constexpr (category == changes_category::intrinsics){
        stable_values.StatusHashes = info.IntrinsicStatusHashes;
        stable_values.ParamValues = info.IntrinsicParamValues;
      } else if constexpr (category == changes_category::current){
        stable_values.StatusHashes = info.CurrentStatusHashes;
        stable_values.ParamValues = info.CurrentParamValues;
      }
      
      return stable_values;
    }
  };
}
