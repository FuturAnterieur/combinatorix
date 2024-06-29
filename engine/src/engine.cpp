#include "engine/include/engine_p.h"
#include "engine/include/geometry_components.h"
#include <chrono>

engine::engine(size_t num_threads, entt::registry *registry) : _Pimpl(new pimpl(num_threads)){
  _Pimpl->Registry = registry;
}

engine::~engine(){
  delete _Pimpl;
}

void engine::launch(){
  _Pimpl->IsRunning = true;
  _Pimpl->ThreadPool.launch();
  post(_Pimpl->ThreadPool, [&]() {
    using namespace std::chrono_literals;
    std::chrono::time_point start = std::chrono::steady_clock::now();
    std::chrono::milliseconds accumulator(0);
    constexpr std::chrono::milliseconds dt{16};
    constexpr float dt_float32 = 0.016;

    while(_Pimpl->IsRunning) {
      std::chrono::time_point after = std::chrono::steady_clock::now();
      std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(after - start);
      start = after;
      accumulator += elapsed;

      while(accumulator >= dt) {
        auto view = _Pimpl->Registry->view<position, velocity>();
        view.each([&](position &p, velocity &v){
          p.x += v.x * dt_float32;
          p.y += v.y * dt_float32;
        });
        accumulator -= dt;
      }
      
      std::this_thread::sleep_for(dt - accumulator);
    };
  });
}

void engine::abort(){
  _Pimpl->IsRunning = false;
  _Pimpl->ThreadPool.abort();
}
