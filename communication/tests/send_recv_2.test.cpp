#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "communication/include/local_communicator.h"
#include "communication/include/local_sync.h"
#include "thread_pool/include/thread_pool.h"
#include <atomic>
#include <chrono>

struct endpoint {
  thread_pool Pool;
  local_communicator Comm;
  endpoint(int num_threads, int comm_index) : Pool(num_threads), Comm(comm_index) {
    
  }
  ~endpoint() = default;
  
};

using namespace std::chrono_literals;

TEST_CASE("bidir with a single channel"){
  endpoint server(1, 0);
  endpoint client(1, 1);

  local_channel_container channels;
  size_t chan_one = channels.add_channel();
  
  server.Comm.set_container(&channels);
  client.Comm.set_container(&channels);

  server.Pool.launch();
  client.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    std::string data = "What is your favorite color?";
    server.Comm.send(chan_one, data);

    std::string answer;
    server.Comm.receive_blocking(chan_one, answer);
    CHECK(answer == "Red");

    IsRunning = false;
  });

  post(client.Pool, [&](){
    std::string question;
    client.Comm.receive_blocking(chan_one, question);
    CHECK(question == "What is your favorite color?");
    client.Comm.send(chan_one, "Red");
  });

  std::this_thread::sleep_for(2s);

  CHECK(IsRunning == false);
}