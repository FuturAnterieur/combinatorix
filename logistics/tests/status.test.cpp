#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "status.h"

TEST_CASE("DLL private vs public"){
  attributes_info power;
  power.CurrentParamValues.emplace(1234, parameter{data_type::boolean, "true"});

  CHECK(true);
}
