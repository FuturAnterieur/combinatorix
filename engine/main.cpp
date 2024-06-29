
//This only helped me realise that structs that only contain data members do not need to be DLL-exported.
//DLL-export only concerns functions or classes that contain methods.

#include "logistics/include/status.h"
#include "logistics/include/effect.h"
#include "engine/include/thread_pool.h"

#include <iostream>
#include <string>
#include <thread>

struct has_name {
  std::string Name;
};

int main(int argc, char *argv[]){
  attributes_info info;
  info.CurrentParamValues.emplace(1234, parameter{data_type::boolean, "Trueest"});
  

  entt::registry dumbo;
  entt::entity ent1 = dumbo.create();
  entt::entity ent2 = dumbo.create();

  dumbo.emplace<attributes_info>(ent1, info);
  dumbo.emplace<has_name>(ent2, "Jordi Savall");

  use(dumbo, ent1, ent2);

  std::cout << dumbo.get<attributes_info>(ent1).CurrentParamValues[1234].Value << "\n";

  thread_pool spa{2};
  using namespace std::chrono_literals;
  std::future<int> waiter = post(spa, use_future([](){std::this_thread::sleep_for(1s); return 1000; }));
  std::cout << waiter.get() << "\n";

  return 0;
}
