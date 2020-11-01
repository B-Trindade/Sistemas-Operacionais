#include <ctime>
#include <time.h>
#include <iostream>

#define MAX_PROCESSES 10

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

int getIODuration(int io) {
  // TODO: modificar os tempos
  switch(io) {
    case DISCO:
      return 100;
    case FITA:
      return 101;
    case IMPRESSORA:
      return 102;
    default:
      return 0;
  }
}

typedef struct IO_Operation {
  int type;             // Identificador do IO
  int start_time;       // Momento de início do IO
} IO_Operation;

typedef struct Process {
  int PID;             // Identificador do processo
  int PPID;            // Identificador do processo pai (sempre = 1)
  int status;          // Status do processo
  int total_time;      // Tempo total a ser executado
  int elapsed_time;    // Tempo já executado
  int priority;        // Nível de prioridade do processo
  IO_Operation* IOs;   // Array de momentos de I/O
} Process;

// Inicializa o array de PIDs com 0s
Process PIDs[MAX_PROCESSES] = {};

// Gera PID novo, distinto de qualquer um presente no array de PIDs
int generatePID() {
  const int MAX_ATTEMPTS = 100;
  int tries = 0;

  bool is_new_pid = false;
  int pid = 0;

  while(!is_new_pid) {
    // Gera PID entre (1,99]
    pid = (std::rand() % 98)+2;

    // Confere se o PID já existe
    is_new_pid = true;
    for(int i = 0; i < MAX_PROCESSES; ++i) {
      // Se o PID já está presente na lista, precisamos gerar outro
      // Sai do `for` e garante outro loop do `while` externo
      if(PIDs[i].PID == pid) {
        is_new_pid = false;
        break;
      }
    }

    // Incrementa número de tentativas de gerar um PID novo
    tries++;
    // Se as tentativas ultrapassarem o limite, retorna valor de -1
    if(tries > MAX_ATTEMPTS)
      return -1;
  }
  return pid;
}

// Função de sleep (em ms)
// [Ref] stackoverflow.com/a/28827188/4824627
void sleep_ms(int milliseconds) {
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

IO_Operation createIO(int type, int start_time) {
  IO_Operation ret;
  ret.type = type;
  ret.start_time = start_time;
  return ret;
}

Process createProcess(int total_time, IO_Operation* ios) {
  Process p;
  p.PID = generatePID();
  p.total_time = total_time;
  p.elapsed_time = 0;
  p.IOs = ios;
  return p;
}


int main() {
  // Determina seed para a função de random
  srand(time(NULL));

  IO_Operation test_ios[3] = {
    createIO(FITA,2),
    createIO(DISCO,1),
    createIO(IMPRESSORA,6)
  };
  Process p = createProcess(10, test_ios);

  std::cout << "Processo de PID " << p.PID << std::endl;
  std::cout << "\tTotal time: " << p.total_time << " u.t." << std::endl;
  std::cout << "\tElapsed time: " << p.elapsed_time << " u.t." << std::endl;

  // sleep_ms(200);
  return 0;
}
