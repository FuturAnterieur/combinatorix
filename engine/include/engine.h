#pragma once


#include "engine/include/engine_export.h"
#include <entt/entity/registry.hpp>

class engine_API engine
{
  public:
    engine(size_t num_threads, entt::registry *registry);
    ~engine();
    void launch();
    void abort();
    //void run_request(/**/);
    
  private:
    struct pimpl;
    pimpl *_Pimpl;
};