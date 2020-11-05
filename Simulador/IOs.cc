#include <time.h>
#include <iostream>

#include "enum.hh"
#include "IOs.hh"

int getIODuration(int io) {
  // TODO: modificar os tempos
  switch(io) {
    case IO_DISCO:
      return 100;
    case IO_FITA:
      return 101;
    case IO_IMPRESSORA:
      return 102;
    default:
      return 0;
  }
}

std::ostream& operator <<(std::ostream& o, const IO_Operation& op) {
  return o << "IO[" << op.type << ";" << op.start_time << "]";
}

std::ostream& operator <<(std::ostream& o, const IO_Operation* arr) {
  if(!arr)
    return o << "(empty)";

  for(int i = 0; i < 50; ++i) {
    if(arr[i].type == -1 || arr[i].start_time == -1)
      break;

    o << arr[i];
  }

  return o;
}

IO_Operation createIO(int type, int start_time) {
  IO_Operation ret;
  ret.type = type;
  ret.start_time = start_time;
  ret.time_left = getIODuration(type);
  return ret;
}

void checkForFinishedIO(int cycle_count) {
  // TODO: essa função
  // TODO: essa funcao deve verificar a 

  (void)cycle_count; // (evita warnings de unused)
  std::cout << "\tConferindo por processos que terminaram IO..." << std::endl;
  std::cout << "\tNenhum processo terminou IO!" << std::endl;
}