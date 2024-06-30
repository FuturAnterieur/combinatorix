#pragma once

#include "engine_client/include/engine_client.h"
#include "thread_pool/include/thread_pool.h"

#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>

struct engine_client::pimpl {
  //IDEA : if local-only, provide option to use a single central registry instead of one for the server and one for each client
  entt::continuous_loader RemoteLoader;
  entt::registry Registry;
  thread_pool ThreadPool;
};