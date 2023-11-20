#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include<functional>


namespace pep::hpc
{
  
  void StartWorkers(int num);
  void StopWorkers();
  
  void RunParallel (int num,
                    const std::function<void(int nr, int size)> & func);
  
}



#endif
