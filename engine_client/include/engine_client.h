#pragma once



struct engine_request_info{
  //Used to go fetch the correct storage pools to place in the snapshot to send to the server.
  std::vector<std::string> affected_status_names;
  std::vector<std::string> affected_param_names;
  std::vector<std::string> affected_true_type_names;
};

class engine;
class engine_client {
private:
  struct pimpl;
  pimpl *_Pimpl;
public:
  //If I do an actual client-server separation one day, "server" will be replaced by a networking interface dealing in byte streams (and not entities/continuous loaders).
  engine_client(int num_threads, const engine &server);
  ~engine_client();

  void launch();

  void transmit_request_to_server(engine_request_info &&info); //will spawn a server request. Called from the game UI.

private:

  void receive_server_updates_thread();

};