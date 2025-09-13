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

TEST_CASE("simple unidir"){
  endpoint server(1, 0);
  endpoint client(1, 1);

  local_channel_container channels;
  size_t chan_idx = channels.add_channel();
  
  server.Comm.set_container(&channels);
  client.Comm.set_container(&channels);

  server.Pool.launch();
  client.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    server.Comm.send(chan_idx, "WOW");
    while(IsRunning){}
  });

  post(client.Pool, [&]() {
    std::string result;
    client.Comm.receive_blocking(chan_idx, result);
    CHECK(result == "WOW");
    IsRunning = false;
  });

  std::this_thread::sleep_for(2s);

  CHECK(!IsRunning);
}

TEST_CASE("simple unidir with receiver delay"){
  endpoint server(1, 0);
  endpoint client(1, 1);

  local_channel_container channels;
  size_t chan_idx = channels.add_channel();
  
  server.Comm.set_container(&channels);
  client.Comm.set_container(&channels);

  server.Pool.launch();
  client.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    std::string data = "Testing 1 2 1 2";
    server.Comm.send(chan_idx, data);
    while(IsRunning){}
  });

  post(client.Pool, [&]() {
    
    std::this_thread::sleep_for(1s);
  
    std::string data;
    client.Comm.receive_blocking(chan_idx, data);
    CHECK(data == "Testing 1 2 1 2");
    IsRunning = false;
  });

  std::this_thread::sleep_for(3s);

  CHECK(!IsRunning);
}

TEST_CASE("simple bidir"){
  endpoint server(1, 0);
  endpoint client(1, 1);

  local_channel_container channels;
  size_t chan_one = channels.add_channel();
  size_t chan_two = channels.add_channel();
  
  server.Comm.set_container(&channels);
  client.Comm.set_container(&channels);

  server.Pool.launch();
  client.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    std::string data = "What is your favorite color?";
    server.Comm.send(chan_one, data);
    //Hmm. since send is asynchronous, I cannot use only one channel
    std::string answer;
    server.Comm.receive_blocking(chan_two, answer);
    CHECK(answer == "Red");

    IsRunning = false;
  });

  post(client.Pool, [&](){
    std::string question;
    client.Comm.receive_blocking(chan_one, question);
    CHECK(question == "What is your favorite color?");
    client.Comm.send(chan_two, "Red");
  });

  std::this_thread::sleep_for(2s);

  CHECK(IsRunning == false);
}

