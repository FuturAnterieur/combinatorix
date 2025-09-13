#include "engine/include/game_communicator.h"
#include "game_communicator.h"

game_communicator::game_communicator(blocking_on_receive_communicator *comm)
{
  _Communication = comm;
}
std::string game_communicator::ask_question(const std::string &question_data, size_t chan_idx)
{
  _Communication->send(chan_idx, question_data);
  std::string answer;
  _Communication->receive(chan_idx, answer);
  return answer;
}