#include <ctime>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "enum.hh"
#include "declares.hh"
#include "Process.hh"
#include "ListController.hh"

// Função de sleep (em ms)
// [Ref] stackoverflow.com/a/28827188/4824627
void sleep_ms(int milliseconds) {
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

// Lista de todos os processos
Process** all_processes;
// Lista de processos com ALTA prioridade
Process** high_processes;
// Lista de processos com BAIXA prioridade
Process** low_processes;
// Listas de processos em IO
Process** io_printer_processes;
Process** io_disk_processes;
Process** io_tape_processes;

ListController high = createLC(high_processes);
ListController low = createLC(low_processes);

ListController io_printer = createLC(io_printer_processes);
ListController io_disk = createLC(io_disk_processes);
ListController io_tape = createLC(io_tape_processes);

// Quando são colocados dentro da função `initializeProcesses()`, essas
// variáveis são descartadas e removidas da memória; assim, o Process fica com
// lixo no lugar :(
IO_Operation IO_Limiter = createIO(-1,-1);
IO_Operation a[3] = { createIO(IO_DISCO,2),createIO(IO_DISCO,3), IO_Limiter };
IO_Operation b[3] = { createIO(IO_IMPRESSORA,4), createIO(IO_FITA,7), IO_Limiter };
IO_Operation c[4] = { createIO(IO_DISCO,1), createIO(IO_FITA,2), createIO(IO_IMPRESSORA,6),IO_Limiter };

// Inicializa processos
void initializeProcesses() {
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    int t = i % 4;
    int random_total_time = (std::rand() % 5)+8;
    int random_start_time = (std::rand() % 5)+1;
    switch(t) {
      case 0: {
        all_processes[i] = new Process(random_total_time, random_start_time, a);
        break;
      } case 1: {
        all_processes[i] = new Process(random_total_time, random_start_time, b);
        break;
      } case 2: {
        all_processes[i] = new Process(random_total_time, random_start_time, c);
        break;
      } case 3: {
        all_processes[i] = new Process(random_total_time, random_start_time);
        break;
      }
    }
    std::cout << "Processo (PID " << all_processes[i]->PID << ") criado: "
              << all_processes[i] << std::endl;
  }
}

ListController& getListForIO(int io_type) {
  switch(io_type) {
    case IO_DISCO:
      return io_disk;
    case IO_FITA:
      return io_tape;
    case IO_IMPRESSORA:
      return io_printer;
    default:
      std::cout << "Não existe IO de tipo " << io_type << "!" << std::endl;
      return io_disk;
  }
}

void executeProcess() {
  Process* proc = all_processes[current_process_index];
  proc->remaining_time--;
  proc->total_time--;
  proc->elapsed_time++;

  std::cout << "\tProcesso " << proc->PID << " falta "
            << proc->total_time << " u.t." << std::endl;


  if(proc->total_time <= 0) {
    std::cout << "\tProcesso terminado!" << std::endl;
    proc->status = STATUS_TERMINATED;
    current_process_index = -1;
    // Remove processo da lista em que ele está
    if(proc->priority == PRIORITY_HIGH)
      shift(high);
    else
      shift(low);
  } else {
    IO_Operation* proc_ios = proc->IOs;
    if(proc_ios) {
      for(int i = 0; i < MAX_IOS; ++i) {
        if(proc_ios[i].type == -1 || proc_ios[i].start_time == -1)
          break;

        if(proc_ios[i].start_time == proc->elapsed_time
        && proc_ios[i].time_left > 0 && !proc_ios[i].done) {
          std::cout << "\tProcesso " << proc->PID << " entrou em I/O! "
                    << "Terminará em " << proc_ios[i].time_left << " u.t."
                    << std::endl;
          current_process_index = -1;
          proc->status = STATUS_WAITING;

          if(proc->priority == PRIORITY_HIGH)
            shift(high);
          else
            shift(low);

          proc->priority = PRIORITY_IO;
          push(getListForIO(proc_ios[i].type), proc);
        }
      }
    }
  }
}

int hasUnfinishedProcess() {
  std::cout << "\tConferindo se ainda existe processo a ser executado" << std::endl;
  // Confere se algum processo não possui status de TERMINATED
  for(int i = 0; i < MAX_PROCESSES; ++i)
    if(all_processes[i]->status != STATUS_TERMINATED)
      return true;
  // Senão, se todos já possuem status TERMINATED
  return false;
}

void checkForPreemption() {
  if(current_process_index < 0)
    return;

  Process* proc = all_processes[current_process_index];

  // Se o timeslice do processo já terminou, e ele ainda está executando
  if(proc->remaining_time <= 0
  && proc->status == STATUS_RUNNING) {
    std::cout << "\tProcesso " << proc->PID
              << " sofreu preempção!" << std::endl;
    // O coloca de volta em espera, com baixa prioridade
    proc->status = STATUS_READY;
    current_process_index = -1;

    if(proc->priority == PRIORITY_HIGH) {
      // Se o processo possuía alta prioridade, remove ele da lista de alta
      // prioridade
      shift(high);
    } else {
      // Senão, remove ele da lista de baixa prioridade para que ele seja
      // reinserido no final dela
      shift(low);
    }
    // Insere o processo na lista de baixa prioridade
    proc->priority = PRIORITY_LOW;
    push(low, proc);
  }
}

void checkForNewProcess(int cycle_count) {
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    Process* proc = all_processes[i];
    // Se o processo inicia nesse ciclo
    if(proc->start_time == cycle_count) {
      // Atualiza seu status
      proc->status = STATUS_READY;
      // E o coloca na lista de alta prioridade
      proc->priority = PRIORITY_HIGH;
      push(high, proc);

      std::cout << "\tProcesso " << proc->PID << " está pronto para ser executado!" << std::endl;
    }
  }
}

void changeProcessPriorityFromIO(Process* proc, int io_type) {
  switch(io_type) {
    case IO_DISCO:
      proc->priority = PRIORITY_LOW;
      push(low, proc);
      break;
    case IO_FITA:
    case IO_IMPRESSORA:
      proc->priority = PRIORITY_HIGH;
      push(high, proc);
      break;
  }
}

void updateIOList(ListController& lc, int io_type) {
  Process* proc = first(lc);
  if(!proc)
    return;
  std::cout << "\tPrimeiro na fila de IO é " << proc->PID << std::endl;

  IO_Operation* proc_ios = proc->IOs;
  if(!proc_ios) {
    std::cout << "\tProcesso não tem lista de IOs! Uh oh!" << std::endl;
    return;
  }

  for(int i = 0; i < MAX_IOS; ++i) {
    IO_Operation& op = proc_ios[i];
    if(op.type == -1 || op.start_time == -1)
      break;

    if(op.type == io_type && !op.done) {
      if(op.time_left > 0) {
        op.time_left--;
        std::cout << "\tProcesso " << proc->PID << " em IO (" << io_type
                  << "), falta " << op.time_left << " u.t." << std::endl;
        return;
      }
      if(op.time_left <= 0) {
        op.done = 1;
        break;
      }
    }
  }

  // Se chegou nesse ponto sem achar nada, quer dizer que o processo deveria
  // sair da lista de I/Os
  shift(lc);
  std::cout << "Processo " << proc->PID << " terminou IO (" << io_type
          << ")!" << std::endl;
  proc->status = STATUS_READY;
  changeProcessPriorityFromIO(proc, io_type);
}

void checkForFinishedIO() {
  std::cout << "\tConferindo por processos que terminaram IO" << std::endl;

  // I/O de disco
  if(io_disk.length > 0)
    updateIOList(io_disk, IO_DISCO);
  // I/O de fita
  if(io_tape.length > 0)
    updateIOList(io_tape, IO_FITA);
  // I/O de impressora
  if(io_printer.length > 0)
    updateIOList(io_printer, IO_IMPRESSORA);
}

int tryRunNewProcess() {
  std::cout << "\tNão há processos executando" << std::endl;
  Process* proc;
  if(high.length > 0) {
    proc = first(high);
  } else if(low.length > 0) {
    proc = first(low);
  } else {
    return -1;
  }
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    if(proc == all_processes[i]) {
      std::cout << "\tExecutando processo " << proc->PID << std::endl;
      proc->status = STATUS_RUNNING;
      proc->remaining_time = TIME_SLICE;
      return i;
    }
  }
  std::cout << "Existe processo na fila, porém ele não está na lista de todos "
            << "os processos?! Isso não deveria acontecer!" << std::endl;
  return -1;
}

void initializeList(Process**& list) {
  list = (Process**)calloc(sizeof(Process*),MAX_PROCESSES);
  if(list == NULL) {
    // Erro criando lista de processos
    std::cout << "calloc falhou!" << std::endl;
    exit(1);
  }
}

std::string getStatusSymbol(int status) {
  switch(status) {
    case STATUS_NEW:
      return "N";
    case STATUS_READY:
      return "R";
    case STATUS_RUNNING:
      return "*";
    case STATUS_WAITING:
      return "W";
    case STATUS_TERMINATED:
      return "T";
    default:
      return "";
  }
}

std::string p(ListController& lc, Process*** pl) {
  std::string str = "[ ";
  for(int i = lc.start_index; i < lc.start_index+lc.length; ++i) {
    Process* proc = (*pl)[i % MAX_PROCESSES];
    str += std::to_string(proc->PID) + getStatusSymbol(proc->status);
    str += " ";
  }
  str += "]";
  // std += "    start=" + std::to_string(lc.start_index);
  // str += "; len=" + std::to_string(lc.length);
  return str;
}
std::string p(int len, Process*** pl) {
  std::string str = "[ ";
  for(int i = 0; i < len; ++i) {
    Process* proc = (*pl)[i];
    str += std::to_string(proc->PID) + getStatusSymbol(proc->status);
    str += " ";
  }
  str += "]";
  return str;
}


void printSystemState() {
  std::cout << "\tEstado das filas:" << std::endl;
  std::cout << "\t\tTODOS:" << std::endl;
  std::cout << "\t\t" << p(MAX_PROCESSES, &all_processes) << std::endl;
  std::cout << "\t\tALTA p.:      " << high.length << " processos" << std::endl;
  std::cout << "\t\t" << p(high, &high_processes) << std::endl;
  std::cout << "\t\tBAIXA p.:     " << low.length << " processos" << std::endl;
  std::cout << "\t\t" << p(low, &low_processes) << std::endl;
  std::cout << "\t\tDISCO=0:      " << io_disk.length << " processos" << std::endl;
  std::cout << "\t\t" << p(io_disk, &io_disk_processes) << std::endl;
  std::cout << "\t\tFITA=1:       " << io_tape.length << " processos" << std::endl;
  std::cout << "\t\t" << p(io_tape, &io_tape_processes) << std::endl;
  std::cout << "\t\tIMPRESSORA=2: " << io_printer.length << " processos" << std::endl;
  std::cout << "\t\t" << p(io_printer, &io_printer_processes) << std::endl;

  std::cout << "\tEstado dos processos:" << std::endl;
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    std::cout << "\t\t" << all_processes[i] << std::endl;
  }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



int main() {
  // Determina seed para a função de random
  srand(time(NULL));

  // Cria as listas de processos
  initializeList(all_processes);
  initializeList(high_processes);
  initializeList(low_processes);
  // Cria listas de I/O
  initializeList(io_printer_processes);
  initializeList(io_disk_processes);
  initializeList(io_tape_processes);

  // Inicializa os processos (hardcoded)
  initializeProcesses();

  sleep_ms(STD_TIMEOUT);
  std::cout << std::endl;
  std::cout << "Todos os processos criados! Iniciando processador..." << std::endl;
  std::cout << std::endl;
  sleep_ms(STD_TIMEOUT*3);

  // Contagem de ciclos do processador
  int global_cycle_count = 0;

  while(true) {
    // Incrementa o ciclo atual
    global_cycle_count++;

    std::cout << std::endl << "Ciclo: " << global_cycle_count << std::endl;

    // TODO: código mais temporário que qualquer outra coisa
    // talvez mudar pra uma função separada seja melhor?
    if(current_process_index >= 0) {
      // Executa um ciclo do processo
      executeProcess();
    } else {
      // Confere se ainda existe algum processo inacabado
      if(!hasUnfinishedProcess())
        // Se não existe, termina o loop
        break;
    }

    sleep_ms(STD_TIMEOUT);

    // Confere se o processo sendo executado deve sofrer preempção
    checkForPreemption();

    sleep_ms(STD_TIMEOUT);

    // Confere se existe algum processo que inicia nesse ciclo
    // (e o coloca na fila apropriada)
    checkForNewProcess(global_cycle_count);

    sleep_ms(STD_TIMEOUT);

    // Confere se algum processo acabou de terminar operação de I/O
    // (e o coloca na fila apropriada)
    checkForFinishedIO();

    sleep_ms(STD_TIMEOUT);

    // (TODO: conferir essa ordem de prioridades)
    if(current_process_index < 0)
      current_process_index = tryRunNewProcess();


    sleep_ms(STD_TIMEOUT);

    printSystemState();

    sleep_ms(STD_TIMEOUT*3);
  }

  std::cout << "Programa terminado!" << std::endl;
  return 0;
}
