#include <iostream>

#ifndef IOs_H  //Include guard
#define IOs_H

typedef struct IO_Operation {
  int type;             // Identificador do IO
  int start_time;       // Momento de início do IO
  int time_left;        // Tempo que falta para IO acabar
} IO_Operation;

int getIODuration(int io);

std::ostream& operator << (std::ostream& o, const IO_Operation& op);
std::ostream& operator << (std::ostream& o, const IO_Operation* arr);

IO_Operation createIO(int type, int start_time);
void checkForFinishedIO(int cycle_count);

#endif /* IOs_H */