#include "local_communicator.h"
#include "local_sync_p.h"

blocking_on_receive_communicator::blocking_on_receive_communicator(int index)
{
  Index = index;
}

queue_on_receive_communicator::queue_on_receive_communicator(int index)
{
  Index = index;
}

void queue_on_receive_communicator::set_container(local_channel_container *container) {
  Container = container;
}

void blocking_on_receive_communicator::set_container(local_channel_container *container)
{
  Container = container;
}

void blocking_on_receive_communicator::send(size_t channel_id, const std::string &data)
{
  local_sync_channel &channel = *Container->get_channel(channel_id);
  std::unique_lock lk(channel.Mutex);
  
  channel.Queue.push_front(data);

  channel.LastMessageCommIndex = Index;
  channel.cvWaiting.notify_one();
  
  lk.unlock();
}

void blocking_on_receive_communicator::receive(size_t channel_id, std::string &data){
  local_sync_channel &channel = *Container->get_channel(channel_id);

  std::unique_lock lk(channel.Mutex);
 
  channel.cvWaiting.wait(lk, [&channel, this](){return !channel.Queue.empty() && channel.LastMessageCommIndex != this->Index; });
  
  //consume data
  //data.insert(data.begin(), std::make_move_iterator(channel.Queue.begin()), std::make_move_iterator(channel.Queue.end()));
  //channel.Queue.clear();

  data = channel.Queue.back();
  channel.Queue.pop_back();
  
  
  lk.unlock();
}

void queue_on_receive_communicator::send(size_t channel_id, const std::string &data){
  //TODO certify this code
  local_sync_channel &channel = *Container->get_channel(channel_id);
  std::unique_lock lk(channel.Mutex);
  
  channel.Queue.push_front(data);

  channel.LastMessageCommIndex = Index;
  channel.cvWaiting.notify_one();
  
  lk.unlock();
}

void queue_on_receive_communicator::receive(size_t channel_id, std::deque<std::string> &data) {
  local_sync_channel &channel = *Container->get_channel(channel_id);

  std::unique_lock lk(channel.Mutex);
  
  //consume data
  if (!channel.Queue.empty() && channel.LastMessageCommIndex != this->Index) {
    data.insert(data.begin(), std::make_move_iterator(channel.Queue.rbegin()), std::make_move_iterator(channel.Queue.rend()));
    channel.Queue.clear();
  }

  lk.unlock();
}