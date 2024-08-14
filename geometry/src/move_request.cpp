#include "move_request.h"
#include "position.h"
#include "velocity.h"

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>

namespace geometry{



  move_request_processor::move_request_processor(entt::registry *registry)
  {
    Registry = registry;
  }

  void move_request_processor::process_move_requests(move_requests_container &reqs, float duration, float delta_time)
  {
    size_t num_steps = static_cast<size_t>(duration / delta_time);
    for(size_t t = 0; t < num_steps; t++){
      for(auto &req : reqs){

        auto &cur_pos = Registry->get<position>(req.Entity);
        glm::vec2 dir_vec = glm::normalize(req.Destination.Value - cur_pos.Value);
        
        glm::vec2 proposed = cur_pos.Value + req.Velocity * delta_time * dir_vec;
        
        //TODO : manage collisions with obstruction colliders (on condition that Entity has an obstruction collider itself)!!!!!
        glm::vec2 reached = proposed;

        cur_pos.Value = reached;
        if(reached != proposed){
          req.Destination.Value = reached;
        }
      }
    }

    reqs.erase(std::remove_if(reqs.begin(),
                              reqs.end(),
                              [=](move_request &req)-> bool 
                              { 
                                auto &cur_pos = Registry->get<position>(req.Entity);
                                const float epsilon = 5e-6;
                                glm::vec2 delta = req.Destination.Value - cur_pos.Value;
                                float delta_squared = delta.x * delta.x + delta.y * delta.y; 
                                return delta_squared <= epsilon;
                              }), 
                    reqs.end());
  }
}