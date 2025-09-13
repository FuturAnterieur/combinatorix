#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "communication/include/local_communicator.h"
#include "communication/include/local_sync.h"
#include "thread_pool/include/thread_pool.h"
#include <atomic>
#include <chrono>

struct endpoint {
  thread_pool Pool;
  blocking_on_receive_communicator Comm;
  endpoint(int num_threads, int comm_index) : Pool(num_threads), Comm(comm_index) {
    
  }
  ~endpoint() = default;
  
};

using namespace std::chrono_literals;

TEST_CASE("bidir with a single channel - simulating game thread + main display thread interaction"){
  endpoint server(1, 0);

  local_channel_container channels;
  size_t chan_one = channels.add_channel();
  size_t chan_two = channels.add_channel();
  
  server.Comm.set_container(&channels);
  
  queue_on_receive_communicator client_comm(1);
  client_comm.set_container(&channels);

  server.Pool.launch();
  std::atomic<bool> IsRunning = true;

  post(server.Pool, [&](){
    server.Comm.send(chan_one, "What is your favorite color?");
    server.Comm.send(chan_one, "Waiting for animation to complete");

    std::string answer;
    server.Comm.receive(chan_two, answer);
    CHECK(answer == "Red");

    server.Comm.receive(chan_two, answer);
    CHECK(answer == "Animation complete");

    IsRunning = false;
  });


  int anim_frame_counter = 0;
  while (IsRunning) {
    std::deque<std::string> received_messages;
    client_comm.receive(chan_one, received_messages);
    while(!received_messages.empty()) {
      std::string data = received_messages.back();
      received_messages.pop_back();
      if (data == "What is your favorite color?") {
        client_comm.send(chan_two, "Red");
      } else if (data == "Waiting for animation to complete") {
        anim_frame_counter = 0;
      }
    }

    std::this_thread::sleep_for(15ms);
    if (++anim_frame_counter == 50) {
      client_comm.send(chan_two, "Animation complete");
    }
  }

  CHECK(IsRunning == false);
}