#include "local_sync.h"
#include "local_sync_p.h"
#include <deque>

struct local_channel_container::pimpl {
  //vector is unusable here, due to local_sync_channel being uncopy-able and unmove-able.
  //another option would be an array... but local_channel_container would have to be templated, so pimpl difficulties. Hmm.
  std::deque<local_sync_channel> Channels;
};

local_channel_container::local_channel_container() : _Pimpl(new pimpl()) {

}

local_channel_container::~local_channel_container() {
  delete _Pimpl;
}

size_t local_channel_container::add_channel(){
  _Pimpl->Channels.emplace_back();
  return _Pimpl->Channels.size() - 1;
}


local_sync_channel *local_channel_container::get_channel(size_t index)
{
  if(index >= _Pimpl->Channels.size()){
    return nullptr;
  }

  return &_Pimpl->Channels[index];
  
}
