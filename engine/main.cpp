
//This only helped me realise that structs that only contain data members do not need to be DLL-exported.
//DLL-export only concerns functions or classes that contain methods.

#include "logistics/include/status.h"
#include "logistics/include/effect.h"
#include "thread_pool/include/thread_pool.h"
#include "engine/include/engine.h"
#include "engine/include/geometry_components.h"

#include <iostream>
#include <string>
#include <thread>

struct has_name {
  std::string Name;
};

int main(int argc, char *argv[]){
  attributes_info info;
  info.CurrentParamValues.emplace(1234, parameter("Trueest"));
  

  entt::registry dumbo;
  entt::entity ent1 = dumbo.create();
  entt::entity ent2 = dumbo.create();

  dumbo.emplace<attributes_info>(ent1, info);
  dumbo.emplace<has_name>(ent2, "Jordi Savall");

  //use(dumbo, ent1, ent2);

  std::cout << std::get<std::string>(dumbo.get<attributes_info>(ent1).CurrentParamValues[1234].value()) << "\n";

  thread_pool spa{2};
  spa.launch();
  using namespace std::chrono_literals;
  std::future<int> waiter = post(spa, use_future([](){std::this_thread::sleep_for(1s); return 1000; }));
  std::cout << waiter.get() << "\n";

  engine moteur(1, &dumbo);
  dumbo.emplace<position>(ent1, 0.0f, 0.f);
  dumbo.emplace<position>(ent2, 5.f, 0.f);

  dumbo.emplace<velocity>(ent1, 0.3f, 0.f);
  dumbo.emplace<velocity>(ent2, -0.3f, 0.f);

  moteur.launch();

  std::this_thread::sleep_for(2s);

  moteur.abort();

  position &pos1 = dumbo.get<position>(ent1);
  std::cout << "x: " << pos1.x << ", y: " << pos1.y << "\n";

  return 0;
}
