#include "local_communicator.h"
#include "local_sync_p.h"

void local_communicator::set_container(local_channel_container *container)
{
  Container = container;
}

void local_communicator::send(size_t channel_id, const std::string &data, bool wait_before, bool wait_after)
{
  local_sync_channel &channel = *Container->get_channel(channel_id);
  std::unique_lock lk(channel.Mutex);
  
  if(wait_before) channel.cvSendCounter.wait(lk, [&channel](){return channel.CompletionCount >= channel.ReceiveCount; });

  channel.InfoReady = true;
  channel.Data = data;

  channel.SendCount++;
  channel.cvWaiting.notify_one();

  if(wait_after) channel.cvSendCounter.wait(lk, [&channel](){return channel.ReceiveCount >= channel.SendCount; });
  channel.CompletionCount++;
  channel.cvSendCounter.notify_all();

  lk.unlock();
}

void local_communicator::receive(size_t channel_id, std::string &data){
  local_sync_channel &channel = *Container->get_channel(channel_id);

  std::unique_lock lk(channel.Mutex);
 
  channel.cvWaiting.wait(lk, [&channel](){return channel.InfoReady; });
  
  //consume data
  data = channel.Data; //use move???
  channel.InfoReady = false;
  channel.ReceiveCount++;
  channel.cvSendCounter.notify_all();

  lk.unlock();
}