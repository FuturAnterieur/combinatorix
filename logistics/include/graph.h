#pragma once 

#include <map>

namespace logistics{
  template<typename NodeIdType>
  class graph{
    public:
      graph() = default;
      ~graph() = default;

      bool add_node(NodeIdType id);
      bool add_edge(NodeIdType from, NodeIdType to);

      bool remove_edge(NodeIdType from, NodeIdType to); //will maybe not be used at all in my case
      bool find_cycle_simple();

    private:
      struct adj_list{
        std::set<NodeIdType> Edges;
      };
      std::map<NodeIdType, adj_list> Nodes;

      bool is_cyclic_util(NodeIdType u, std::map<NodeIdType, bool> &visited, std::map<NodeIdType, bool> &active);
  };

  template<typename NodeIdType>
  inline bool graph<NodeIdType>::add_node(NodeIdType id){
    return Nodes.emplace(id, adj_list{}).second;
  }

  template<typename NodeIdType>
  inline bool graph<NodeIdType>::add_edge(NodeIdType from, NodeIdType to){
    adj_list &from_data = Nodes.emplace(from, adj_list{}).first->second;
    Nodes.emplace(to, adj_list{});
    return from_data.Edges.emplace(to).second;
  }
  
  template<typename NodeIdType>
  inline bool graph<NodeIdType>::remove_edge(NodeIdType from, NodeIdType to){
    adj_list &from_data = Nodes.emplace(from, adj_list{}).first->second;
    return from_data.Edges.erase(to) > 0;
  }

  template<typename NodeIdType>
  inline bool graph<NodeIdType>::find_cycle_simple(){
    std::map<NodeIdType, bool> visited;
    std::map<NodeIdType, bool> active;

  
    for(const auto &[id, adj_list] : Nodes){
      visited[id] = false;
      active[id] = false;
    }

    for(const auto &[id, adj_list] : Nodes){
      if(!visited[id] && is_cyclic_util(id, visited, active)){
        return true;
      }
    }

    return false;
  }

  template<typename NodeIdType>
  inline bool graph<NodeIdType>::is_cyclic_util(NodeIdType u, std::map<NodeIdType, bool> &visited, std::map<NodeIdType, bool> &active){
    if(visited[u] == false){
      visited[u] = true;
      active[u] = true;

      for(NodeIdType v : Nodes.at(u).Edges){
        if(!visited[v] && is_cyclic_util(v, visited, active)){
          return true;
        } else if(active[v]) {
          return true;
        }
      }
    }
    active[u] = false;
    return false;
  }

}
