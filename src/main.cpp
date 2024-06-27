
#include "split.h"
#include "effect.h"
#include "combine.h"
#include "status.h"
#include <entt/entity/registry.hpp>
#include <iostream>
#include <string>

struct has_name {
  std::string Name;
};

int main(int argc, char* argv[]){

  entt::registry registry;

  split_rule my_rule;
  my_rule.TypeGroups.push_back(std::set<entt::type_info>({entt::type_id<has_name>()}));
  my_rule.TypeGroups.push_back(std::set<entt::type_info>({entt::type_id<combination_info>()}));

  std::vector<entt::entity> split_result;
  //split(registry, la_couverte, my_rule, split_result);

  std::cout << "split result : ";
  auto &combines = registry.get<has_name>(split_result[0]).Name;
  std::cout << combines << "\n";

  return 0;
}