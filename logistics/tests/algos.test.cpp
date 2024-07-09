#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "graph.h"
#include "floyd.h"


TEST_CASE("Simple floyd"){
  std::vector<int> test1 = {0,1,2,3,0,1,2,3};
  std::vector<int> test2 = {0,1,4,5,6,4,5,6};
  std::vector<int> test3 = {0,1,4,3,5,7,1};
  std::vector<int> test4 = {0,1,2,3,4,5,6};

  size_t start, end;
  CHECK(floyd::find_cycle(test1, 0, start, end)); CHECK(start == 0); CHECK(end == 4);
  CHECK(floyd::find_cycle(test2, 0, start, end)); CHECK(start == 2); CHECK(end == 5);
  CHECK_FALSE(floyd::find_cycle(test3, 0, start, end));
  CHECK_FALSE(floyd::find_cycle(test4, 0, start, end));

}