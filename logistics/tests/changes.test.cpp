#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "attributes_info.h"

TEST_CASE("reverse changes -- do they work????!?!?!") {
  attributes_info_snapshot snapshot1, snapshot2;
  snapshot1.ParamValues.Values.emplace(1234, "Hello");
  snapshot1.StatusHashes.Values.emplace(4321, status_t{true});

  snapshot2.ParamValues.Values.emplace(1234, "Goodbye");
  snapshot2.StatusHashes.Values.emplace(5678, status_t{true});

  attributes_info_changes reverse_changes = compute_diff(snapshot2, snapshot1);
  CHECK(reverse_changes.ModifiedStatuses.Changes.at(4321).Change.Diff == smt::added);
  CHECK(reverse_changes.ModifiedStatuses.Changes.at(5678).Change.Diff == smt::removed);
  
  attributes_info_changes forward_changes = compute_diff(snapshot1, snapshot2);
  CHECK(forward_changes.ModifiedStatuses.Changes.at(5678).Change.Diff == smt::added);
  CHECK(forward_changes.ModifiedStatuses.Changes.at(4321).Change.Diff == smt::removed);
}
