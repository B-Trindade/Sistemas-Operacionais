#include <time.h>
#include <iostream>

#include "enum.hh"
#include "declares.hh"
#include "IOs.hh"

int getIODuration(int io) {
  // TODO: modificar os tempos
  switch(io) {
    case IO_DISCO:
      return 2;
    case IO_FITA:
      return 5;
    case IO_IMPRESSORA:
      return 10;
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

  for(int i = 0; i < MAX_IOS; ++i) {
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
  ret.done = 0;
  return ret;
}