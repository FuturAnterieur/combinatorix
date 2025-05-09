#pragma once

#include "attributes_info.h"
#include "history_utils.h"
#include "entt_utils.h"

namespace logistics{

  template<changes_category category, typename snapshot_t = attributes_info_snapshot, typename history_t = attributes_info_history>
  struct history_storage {
    private:
      entt::id_type ChangesBranchHash; //Branches of attributes_info_history
      entt::id_type StableBranchHash; //Branches of attributes_info_snapshot (will replace present unique storage of attributes_info for both intrinsic and current)
      entt::registry *_Registry;

    public:
    history_storage() = default;
    ~history_storage() = default;

    inline void set_registry(entt::registry *registry){
      _Registry = registry;
    }

    inline void set_branch_name(const std::string &name){
      std::string long_string_changes = name + std::to_string(static_cast<int>(category)) + std::string(entt::type_name<attributes_info_history>().value());
      ChangesBranchHash = entt::hashed_string::value(long_string_changes.data());
      std::string long_string_stable = name + std::to_string(static_cast<int>(category)) + std::string(entt::type_name<attributes_info_snapshot>().value());
      StableBranchHash = entt::hashed_string::value(long_string_stable.data());
    }

    inline void init_history_starting_point(entt::entity entity){
      auto &storage = get_changes_storage();
      if(!storage.contains(entity)){
        auto &stable_snapshot = get_stable_storage().get(entity);
        storage.emplace(entity, stable_snapshot);
      }
    }

    inline void init_history_starting_point(entt::entity entity, const attributes_info_snapshot &snapshot){
      auto &storage = get_changes_storage();
      if(!storage.contains(entity)){
        storage.emplace(entity, snapshot);
      }
    }

    inline void set_stable_values(entt::entity entity, const attributes_info_cumulative_changes &changes){
      auto &stable_values = utils::get_or_emplace<snapshot_t>(*_Registry, entity, StableBranchHash);
      paste_cumulative_changes(changes, stable_values);
    }
    
    inline void add_changes(entt::entity entity, const attributes_info_cumulative_changes &changes, timing_t timing){
      auto &status_changes_storage = get_changes_storage();
      init_history_starting_point(entity);
      auto &history = status_changes_storage.get(entity);
      history.add_changes(timing, changes);
    }

    inline void commit_timing(entt::entity entity, timing_t timing, bool apply_to_registry = false){
      auto &status_changes_storage = get_changes_storage();
      init_history_starting_point(entity);

      auto &history = status_changes_storage.get(entity);

      priority_callback cb;
      cb.Func = classic_priority_callback;
      cb.UserData = _Registry;

      history.merge_timing(timing, cb);
      if (apply_to_registry){
        //TODO what to do in case of merge refusal??
        paste_changes_to_official_registry(_Registry, history.cumulative_changes(timing), entity);
      }
    }

    inline void merge_to_reality(timing_t upper_bound, bool apply_to_registry = false){
      auto &changes_storage = get_changes_storage();
      auto status_changes_view = entt::view<entt::get_t<attributes_info_history>>{changes_storage};

      for(entt::entity entity : status_changes_view){
        auto &history = status_changes_view.get<attributes_info_history>(entity);

        
        attributes_info_cumulative_changes cumul = history.cumulative_changes(upper_bound);

        paste_cumulative_changes(cumul, get_stable_storage().get(entity));

        if (apply_to_registry){
          paste_changes_to_official_registry(_Registry, cumul, entity);
        }
      }
      changes_storage.clear();
    }

    inline status_changes_storage_t &get_changes_storage(){
      return _Registry->storage<attributes_info_history>(ChangesBranchHash);
    }

    inline status_stable_storage_t &get_stable_storage() {
      return _Registry->storage<snapshot_t>(StableBranchHash);
    }

    inline attributes_info_snapshot get_most_recent_snapshot(entt::entity entity) {
      auto &storage = get_changes_storage();
      if(storage.contains(entity)){
        
        auto &history = storage.get(entity);
        return history.produce_snapshot(); 
      }

      return  get_stable_storage().get(entity);
    }

    inline void clear_storages(){
      get_changes_storage().clear();
      get_stable_storage().clear();
    }
  };
}
