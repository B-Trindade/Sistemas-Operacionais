#include <iostream>
#include <time.h>

#include "enum.hh"
#include "declares.hh"
#include "Process.hh"

int current_process_index = -1;
int process_count = 0;

// Operador << para impressão de ponteiros de Process
std::ostream& operator <<(std::ostream& o, const Process* p) {
  return o << "[*Process](PID=" << p->PID << ")"
           << "(start=" << p->start_time << ")"
           << "(remaining=" << p->remaining_time << ")"
           << "(total=" << p->total_time << ")"
           << "(IOs=" << p->IOs << ")";
}

// Operador << para impressão de referências de Process
std::ostream& operator <<(std::ostream& o, const Process& p) {
  return o << "[Process](PID=" << p.PID << ")"
           << "(start=" << p.start_time << ")"
           << "(remaining=" << p.remaining_time << ")"
           << "(total=" << p.total_time << ")"
           << "(IOs=" << p.IOs << ")";
}
