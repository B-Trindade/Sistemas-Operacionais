#ifndef ENUM_H  //Include guard
#define ENUM_H

enum IOTypes {
  IO_DISCO = 0,
  IO_FITA,
  IO_IMPRESSORA
};

enum Priorities {
  PRIORITY_HIGH = 0,
  PRIORITY_LOW,
  PRIORITY_IO
};

enum Status {
  STATUS_NEW = 0,   // Foi criado, ainda não está na fila
  STATUS_READY,     // Esperando ser executado
  STATUS_RUNNING,   // Está sendo executado
  STATUS_WAITING,   // Entrou em IO / realizando IO
  STATUS_TERMINATED // Processo acabou
};

#endif /* ENUM_H */