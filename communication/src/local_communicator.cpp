#include "local_communicator.h"
#include "local_sync_p.h"

local_communicator::local_communicator(int index)
{
  Index = index;
}

void local_communicator::set_container(local_channel_container *container)
{
  Container = container;
}

void local_communicator::send(size_t channel_id, const std::string &data)
{
  local_sync_channel &channel = *Container->get_channel(channel_id);
  std::unique_lock lk(channel.Mutex);
  
  channel.InfoReady = true;
  channel.Data = data;

  channel.LastMessageCommIndex = Index;
  channel.cvWaiting.notify_one();
  
  lk.unlock();
}

void local_communicator::receive(size_t channel_id, std::string &data){
  local_sync_channel &channel = *Container->get_channel(channel_id);

  std::unique_lock lk(channel.Mutex);
 
  channel.cvWaiting.wait(lk, [&channel, this](){return channel.InfoReady && channel.LastMessageCommIndex != this->Index; });
  
  //consume data
  data = channel.Data; //use move???
  channel.InfoReady = false;
  
  
  lk.unlock();
}