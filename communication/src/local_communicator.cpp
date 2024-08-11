#include "local_communicator.h"

void local_communicator::request(local_sync_channel &channel){
  std::unique_lock lk(channel.Mutex);
  channel.InfoReady = false;
  channel.cvAnswerer.notify_one();
  channel.cvRequester.wait(lk, [&channel](){return channel.InfoReady; });

  lk.unlock();
}

void local_communicator::respond(local_sync_channel &channel){
  std::unique_lock lk(channel.Mutex);
  channel.InfoReady = true;
  channel.cvRequester.notify_one();
  channel.cvAnswerer.wait(lk, [&channel](){return !channel.InfoReady; });

  lk.unlock();
}