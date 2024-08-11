#include "local_communicator.h"
#include "local_sync_p.h"

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
  channel.cvWaiting.notify_one();
  lk.unlock();
}

void local_communicator::send_and_wait_for_ack(size_t channel_id, const std::string &data){
  local_sync_channel &channel = *Container->get_channel(channel_id);
  
  std::unique_lock lk(channel.Mutex);
  channel.InfoReady = true;
  channel.Data = data;
  channel.cvWaiting.notify_one();
  channel.ReceiverAck = false;
  channel.cvReceiverAck.wait(lk, [&channel](){return channel.ReceiverAck; });
  lk.unlock();
}

void local_communicator::receive(size_t channel_id, std::string &data){
  local_sync_channel &channel = *Container->get_channel(channel_id);

  std::unique_lock lk(channel.Mutex);
 
  channel.cvWaiting.wait(lk, [&channel](){return channel.InfoReady; });
  channel.ReceiverAck = true;
  channel.cvReceiverAck.notify_one(); //in case someone is waiting

  //consume data
  data = channel.Data; //use move???
  channel.InfoReady = false;

  lk.unlock();
}