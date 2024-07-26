#pragma once

#include "logistics_export.h"
#include "attributes_info.h"
#include <entt/entity/registry.hpp>
#include <functional>
#include <list>
#include <set>
#include <map>


//Keep entity lists in logistics (entities having components created in engine-server, that do contain the user-funcs)
struct status_effects_affecting { //i.e. generally status effects currently applying to the parent entity
  std::list<entt::entity> EffectEntities;
  //entt::constness_as_t<entt::storage_type_t<effect_info, entt::entity, std::allocator<effect_info>>, effect_info> InfosOnSteroids;
};
using sea_state_at_timing = status_effects_affecting;

struct status_effects_affecting_history {
  std::map<timing_t, sea_state_at_timing> History;
};

struct status_effects_owned{
  std::list<entt::entity> EffectEntities;
};

//Active effects, Passive effects
//Passive : Status modifying effects -> reran each time they are modified
//Effects that stay on place but do something at regular time intervals / according to certain events -> use a trigger system
//Active : are called only on use
