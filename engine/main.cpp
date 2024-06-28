
//This only helped me realise that structs that only contain data members do not need to be DLL-exported.
//DLL-export only concerns functions or classes that contain methods.

#include "logistics/include/status.h"
#include "logistics/include/effect.h"

#include <iostream>

int main(int argc, char *argv[]){
  attributes_info info;
  info.CurrentParamValues.emplace(1234, parameter{data_type::boolean, "Trueest"});
  std::cout << info.CurrentParamValues[1234].Value << "\n";

  entt::registry dumbo;
  entt::entity ent1, ent2;

  use(dumbo, ent1, ent2);

  return 0;
}
