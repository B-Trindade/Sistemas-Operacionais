#include <iostream>

#include "declares.hh"
#include "Process.hh"
#include "ListController.hh"

ListController createLC(Process**& list) {
  ListController ret;
  ret.start_index = 0;
  ret.length = 0;
  ret.list = &list;
  return ret;
}


Process* first(ListController& lc) {
  return (*lc.list)[lc.start_index];
}

void push(ListController& lc, Process* proc) {
  lc.length++;
  int index = (lc.start_index + lc.length - 1) % MAX_PROCESSES;
  std::cout << "\tInserindo processo " << proc->PID << " em [" << index << "] ("
            << lc.length << ")" << std::endl;
  (*lc.list)[index] = proc;
}

Process* shift(ListController& lc) {
  Process* first = (*lc.list)[lc.start_index];
  lc.start_index = (lc.start_index + 1) % MAX_PROCESSES;
  lc.length--;
  std::cout << "\tRemovendo processo " << first->PID << " (" << lc.length << ")" << std::endl;
  return first;
}