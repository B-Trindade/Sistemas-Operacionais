#include <iostream>
#include <time.h>

#include "enum.hh"
#include "declares.hh"
#include "Process.hh"

int current_process_index = -1;
int process_count = 0;

// Operador << para impressão de ponteiros de Process
std::ostream& operator <<(std::ostream& o, const Process* p) {
  if(!p->total_time)
    return o << "[Process " << p->PID << "] done";

  return o << "[Process " << p->PID << "] "
           << "priority=" << p->priority << ";"
           << "start=" << p->start_time << ";"
           << "total=" << p->total_time << ";"
           << "slice=" << p->remaining_time << ";"
           << "IOs=" << p->IOs;
}