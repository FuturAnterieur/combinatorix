#pragma once


#include "engine_export.h"

struct engine_pimpl;
class engine_API engine
{
  public:
    engine();
    ~engine();
    void launch();
    void run();
    void joinThread();
    
    bool isFinished();
    bool isInfoRequired();

  private:
    engine_pimpl *_Pimpl;
    void engine_thread();
    
};