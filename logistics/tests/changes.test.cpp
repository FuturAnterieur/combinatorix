#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "attributes_info.h"
#include "status.h"

TEST_CASE("reverse changes -- do they work????!?!?!") {
  attributes_info_snapshot snapshot1, snapshot2;
  snapshot1.ParamValues.emplace(1234, "Hello");
  snapshot1.StatusHashes.emplace(4321);

  snapshot2.ParamValues.emplace(1234, "Goodbye");
  snapshot2.StatusHashes.emplace(5678);

  attributes_info_changes reverse_changes = compute_diff(snapshot2, snapshot1);
  CHECK(reverse_changes.ModifiedStatuses.at(4321) == smt::added);
  CHECK(reverse_changes.ModifiedStatuses.at(5678) == smt::removed);
  
  attributes_info_changes forward_changes = compute_diff(snapshot1, snapshot2);
  CHECK(forward_changes.ModifiedStatuses.at(5678) == smt::added);
  CHECK(forward_changes.ModifiedStatuses.at(4321) == smt::removed);
}
