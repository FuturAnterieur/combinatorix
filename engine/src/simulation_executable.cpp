#include "simulation_executable.h"
#include "game_logic.h"

namespace engine{
  void status_trigger_executable::operator()(){
    eng->set_context_originating_entity(Info.TriggerOwner);
    Info.Func(eng, Changes, TriggeringEntity, Info);
    eng->set_context_originating_entity(entt::null);
  }

  void entity_update_executable::operator()() {
    eng->update_status(EntityToUpdate);
  }
}
