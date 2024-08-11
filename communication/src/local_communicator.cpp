#include "local_communicator.h"
#include "local_sync_p.h"

void local_communicator::set_container(local_channel_container *container)
{
  Container = container;
}

void local_communicator::wait_receiver_ready(size_t channel_id){
  local_sync_channel &channel = *Container->get_channel(channel_id);
  std::unique_lock lk(channel.Mutex);
  channel.cvReceiverReady.wait(lk, [&channel](){return channel.ReceiverReady; });
  lk.unlock();
}


void local_communicator::send(size_t channel_id, const std::string &data)
{
  local_sync_channel &channel = *Container->get_channel(channel_id);
  std::unique_lock lk(channel.Mutex);
  channel.InfoReady = true;
  channel.Data = data;
  channel.cvWaiting.notify_one();
  lk.unlock();
}

void local_communicator::receive(size_t channel_id, std::string &data){
  local_sync_channel &channel = *Container->get_channel(channel_id);

  std::unique_lock lk(channel.Mutex);
 
  channel.ReceiverReady = true;
  channel.cvReceiverReady.notify_one(); //in case someone is waiting
  channel.cvWaiting.wait(lk, [&channel](){return channel.InfoReady; });
  channel.ReceiverReady = false;
  
  //consume data
  data = channel.Data; //use move???
  channel.InfoReady = false;

  lk.unlock();
}