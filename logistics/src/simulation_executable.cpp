#include "simulation_executable.h"
#include "simulation_engine.h"

namespace logistics{
  void status_trigger_executable::operator()(entt::registry &registry, simulation_engine *eng){
    eng->ChangesContext.OriginatingEntity = Info.TriggerOwner;
    Info.Func(registry, Changes, TriggeringEntity, Info);
    eng->ChangesContext.OriginatingEntity = entt::null;
  }


  void entity_update_executable::operator()(entt::registry &registry, simulation_engine *eng) {
    update_status_effects(registry, EntityToUpdate);
  }
}
