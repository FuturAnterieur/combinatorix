#pragma once

#include "engine_export.h"
#include "engine.h"

namespace entt{
  class registry;
}

class engine_API engine_builder {
public:
  engine_builder();
  ~engine_builder();
  
  void set_registry(entt::registry *registry);
  void set_num_threads(size_t num);
  bool finalize(engine *eng);

private:
  struct pimpl;
  pimpl *_Pimpl;

};