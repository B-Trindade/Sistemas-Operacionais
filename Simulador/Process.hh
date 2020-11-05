#include "enum.hh"
#include "IOs.hh"
#include "declares.hh"

#ifndef PROCESS_H
#define PROCESS_H

extern int current_process_index;
extern int process_count;

class Process {
public:
  int PID;                      // Identificador do processo
  int PPID = 1;                 // Identificador do processo pai (sempre = 1)
  int status = STATUS_NEW;      // Status do processo
  int priority = PRIORITY_HIGH; // Nível de prioridade do processo
  int total_time;               // Tempo total a ser executado
  int start_time;               // Momento em que o processo inicia
  int remaining_time = 0;       // Tempo já executado (nesse timeslice)
  IO_Operation* IOs;            // Array de momentos de I/O

public:
  // Caso só exista parâmetro de tempo total, assumimos que não há I/O,
  // e que o processo inicia imediatamente
  Process(int total_time):
    PID( generatePID() ),
    total_time(total_time),
    start_time(0) {}

  Process(int total_time, int start_time):
    PID( generatePID() ),
    total_time(total_time),
    start_time(start_time) {}

  Process(int total_time, IO_Operation* ios):
    PID( generatePID() ),
    total_time(total_time),
    start_time(0),
    IOs(ios) {}

  Process(int total_time, int start_time, IO_Operation* ios):
    PID( generatePID() ),
    total_time(total_time),
    start_time(start_time),
    IOs(ios) {}

private:
  // Gera PID novo, exclusivo para o processo
  int generatePID() {
    // Se o número de processos ainda não ultrapassou o limite
    if(process_count < MAX_PROCESSES) {
      // Aumenta o número de processos
      process_count++;
      // Retorna o valor
      return process_count;
    }
    return -1;
  }
};

// Operador << para impressão de ponteiros de Process
std::ostream& operator << (std::ostream& o, const Process* p);
// Operador << para impressão de referências de Process
std::ostream& operator << (std::ostream& o, const Process& p);

#endif /* PROCESS_H */