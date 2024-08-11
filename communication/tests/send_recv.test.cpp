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
  endpoint(int num_threads) : Pool(num_threads) {
    
  }
  ~endpoint() = default;
  
};


TEST_CASE("simple unidir"){
  endpoint server(1);
  endpoint client(1);

  local_channel_container channels;
  size_t chan_idx = channels.add_channel();
  
  server.Comm.set_container(&channels);
  client.Comm.set_container(&channels);

  server.Pool.launch();
  client.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    server.Comm.wait_receiver_ready(chan_idx);
    server.Comm.send(chan_idx);
    while(IsRunning){}
  });

  using namespace std::chrono_literals;
  post(client.Pool, [&]() {
    
    std::this_thread::sleep_for(2s);
  
    client.Comm.receive(chan_idx);
    CHECK(true);
    IsRunning = false;
  });

  while(IsRunning){}

  CHECK(true);
}

TEST_CASE("simple unidir without waiting for receiver-ready"){
  endpoint server(1);
  endpoint client(1);

  local_channel_container channels;
  size_t chan_idx = channels.add_channel();
  
  server.Comm.set_container(&channels);
  client.Comm.set_container(&channels);

  server.Pool.launch();
  client.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    server.Comm.send(chan_idx);
    while(IsRunning){}
  });

  using namespace std::chrono_literals;
  post(client.Pool, [&]() {
    
    std::this_thread::sleep_for(2s);
  
    client.Comm.receive(chan_idx);
    CHECK(true);
    IsRunning = false;
  });

  while(IsRunning){}

  CHECK(true);
}