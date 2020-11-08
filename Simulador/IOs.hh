#include <iostream>
#include <string>

#ifndef IOs_H  //Include guard
#define IOs_H

typedef struct IO_Operation {
  int type;             // Identificador do IO
  int start_time;       // Momento de início do IO
  int time_left;        // Tempo que falta para IO acabar
  int done;             // Se o IO já foi realizado
} IO_Operation;

int getIODuration(int io);
std::string getIOName(int io_type);

std::ostream& operator << (std::ostream& o, const IO_Operation& op);
std::ostream& operator << (std::ostream& o, const IO_Operation* arr);

IO_Operation createIO(int type, int start_time);

#endif /* IOs_H */