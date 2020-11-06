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


// Controladores das filas de prioridades
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

// Função para pegar o controlador da fila dado um tipo de IO
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

// Função para executar um ciclo do processo
void executeProcess() {
  // Pega referência ao processo
  Process* proc = all_processes[current_process_index];
  // Atualiza: o tempo que falta nesse timeslice,
  proc->remaining_time--;
  // o tempo que falta de execução no total,
  proc->total_time--;
  // e o tempo já executado pelo processo
  proc->elapsed_time++;

  // Avisa ao usuário
  std::cout << "\tExecutando processo " << proc->PID << "; falta "
            << proc->total_time << " u.t." << std::endl;

  // Caso o procesos tenha terminado
  if(proc->total_time <= 0) {
    // Avisa ao usuário
    std::cout << "\tProcesso " << proc->PID << " terminado!" << std::endl;
    // Atualiza o status do processo para "terminado"
    proc->status = STATUS_TERMINATED;
    // Apaga index do processo para que outro possa ser executado
    current_process_index = -1;
    // Remove processo da lista em que ele está
    if(proc->priority == PRIORITY_HIGH) {
      std::cout << "\tRemovendo " << proc->PID
                << " da fila de alta prioridade" << std::endl;
      shift(high);
    } else {
      std::cout << "\tRemovendo " << proc->PID
                << " da fila de baixa prioridade" << std::endl;
      shift(low);
    }
    // Vai embora, processo já terminou
    return;
  }

  // Caso contrário, se o processo não terminou
  // Confere se ele possui lista de I/Os
  IO_Operation* proc_ios = proc->IOs;
  if(proc_ios) {
    // Caso possua, itera sobre a lista de I/Os dele
    for(int i = 0; i < MAX_IOS; ++i) {
      int io_type = proc_ios[i].type;

      // Se chegamos ao fim da lista, sai
      if(io_type == -1 || proc_ios[i].start_time == -1)
        break;

      // Caso exista um I/O com tempo de início igual ao tempo já executado pelo
      // processo, e que ainda não terminou
      if(proc_ios[i].start_time == proc->elapsed_time
      && proc_ios[i].time_left > 0 && !proc_ios[i].done) {
        // Entra em I/O
        std::cout << "\tProcesso " << proc->PID << " entrou em I/O! "
                  << "Precisa de " << proc_ios[i].time_left << " u.t. de I/O"
                  << std::endl;
        // Atualiza o status do processo para "esperando" (status de I/O)
        proc->status = STATUS_WAITING;
        // Apaga index do processo para que outro possa ser executado enquanto
        // esse espera o I/O
        current_process_index = -1;

        // Remove o processo da fila de prioridade em que estava
        if(proc->priority == PRIORITY_HIGH) {
          std::cout << "\tRemovendo " << proc->PID
                    << " da fila de alta prioridade" << std::endl;
          shift(high);
        } else {
          std::cout << "\tRemovendo " << proc->PID
                    << " da fila de baixa prioridade" << std::endl;
          shift(low);
        }

        // Atualiza a prioridade do processo para prioridade especial de I/O
        proc->priority = PRIORITY_IO;
        // Adiciona o processo à lista adequada de I/O
        std::cout << "\tAdicionando " << proc->PID << " à fila de I/O ("
                  << getIOName(io_type) << ")" << std::endl;
        push(getListForIO(io_type), proc);
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
  // Caso não haja processo executando, sai
  if(current_process_index < 0)
    return;

  // Pega referência ao processo sendo executado
  Process* proc = all_processes[current_process_index];

  // Se o timeslice do processo já terminou, e ele ainda está executando
  if(proc->remaining_time <= 0
  && proc->status == STATUS_RUNNING) {
    std::cout << "\tProcesso " << proc->PID
              << " sofreu preempção!" << std::endl;
    // O coloca de volta em espera, com baixa prioridade
    proc->status = STATUS_READY;
    // Apaga index do processo para que outro possa ser executado
    current_process_index = -1;

    // Remove o processo da fila de prioridade em que estava
    if(proc->priority == PRIORITY_HIGH) {
      std::cout << "\tRemovendo " << proc->PID
                << " da fila de alta prioridade" << std::endl;
      shift(high);
    } else {
      std::cout << "\tRemovendo " << proc->PID
                << " da fila de baixa prioridade" << std::endl;
      shift(low);
    }

    // Insere o processo na lista de baixa prioridade
    proc->priority = PRIORITY_LOW;
    std::cout << "\tAdicionando " << proc->PID
              << " à fila de baixa prioridade" << std::endl;
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

      std::cout << "\tAdicionando " << proc->PID
                << " à fila de alta prioridade" << std::endl;
    }
  }
}

// Função que adiciona o processo à fila de prioridade correta dependendo do I/O
void changeProcessPriorityFromIO(Process* proc, int io_type) {
  // Por definição do trabalho, após I/O de disco, o processo retorna à fila de
  // baixa prioridade; para outros tipos de I/O, retorna à fila de alta
  // prioridade
  switch(io_type) {
    case IO_DISCO:
      proc->priority = PRIORITY_LOW;
      push(low, proc);
      std::cout << "\t| Adicionando " << proc->PID
                << " à fila de baixa prioridade" << std::endl;
      break;
    case IO_FITA:
    case IO_IMPRESSORA:
      proc->priority = PRIORITY_HIGH;
      push(high, proc);
      std::cout << "\t| Adicionando " << proc->PID
                << " à fila de alta prioridade" << std::endl;
      break;
  }
}

// Função que atualiza uma lista de I/O, dado seu controlador e tipo de I/O
void updateIOList(ListController& lc, int io_type) {
  // Pega referência ao primeiro processo na fila
  Process* proc = first(lc);
  // Se não existe processo na file, retorna (lista está vazia)
  if(!proc)
    return;
  // Senão, imprime para o usuário o PID do processo
  std::cout << "\t| Primeiro na fila de I/O " << getIOName(io_type)
            << " é Processo " << proc->PID << std::endl;

  // Pega lista de I/Os do processo
  IO_Operation* proc_ios = proc->IOs;
  // Se não há lista de I/Os, algo deu errado!!
  if(!proc_ios) {
    std::cout << "\t| Processo não tem lista de IOs! Uh oh!" << std::endl;
    return;
  }

  // Itera sobre a lista de I/Os do processo
  for(int i = 0; i < MAX_IOS; ++i) {
    IO_Operation& op = proc_ios[i];
    // Se chegamos ao final da lista, sai
    if(op.type == -1 || op.start_time == -1)
      break;

    // Caso o i-ésimo I/O seja do tipo especificado, não tenha terminado...
    if(op.type == io_type && !op.done) {
      // E ainda precise de mais tempo
      if(op.time_left > 0) {
        // Diminui uma unidade de tempo
        op.time_left--;
        // Avisa ao usuário, e sai da função
        std::cout << "\t| Processo " << proc->PID << " em I/O "
                  << getIOName(io_type) << ", falta " << (op.time_left+1)
                  << " u.t." << std::endl;
        return;
      }
      // Caso contrário, se ele não precisa de mais tempo
      if(op.time_left <= 0) {
        // Marca como terminado e continua abaixo
        op.done = 1;
        break;
      }
    }
  }

  // Se chegou nesse ponto:
  // - ou o processo terminou seu I/O
  // - ou o programa não encontrou I/O nenhum do tipo especificado
  // Em ambos os casos, o processo deveria sair da lista de I/Os
  shift(lc);
  // Avisa ao usuário
  std::cout << "\t| Processo " << proc->PID << " terminou I/O ("
            << getIOName(io_type) << ")! Removendo da fila de I/Os"
            << std::endl;
  // Modifica o status do processo
  proc->status = STATUS_READY;
  // Adiciona o processo à fila de prioridade adequada (depende do tipo de I/O)
  changeProcessPriorityFromIO(proc, io_type);
}

void checkForFinishedIO() {
  std::cout << "\tConferindo por processos que terminaram I/O:" << std::endl;

  // I/O de disco
  if(io_disk.length > 0)
    updateIOList(io_disk, IO_DISCO);
  // I/O de fita
  if(io_tape.length > 0)
    updateIOList(io_tape, IO_FITA);
  // I/O de impressora
  if(io_printer.length > 0)
    updateIOList(io_printer, IO_IMPRESSORA);

  std::cout << "\tConferido!" << std::endl;
}

int tryRunNewProcess() {
  std::cout << "\tNão há processos executando" << std::endl;

  Process* proc;
  // Caso a lista de alta prioridade possua algum processo
  if(high.length > 0) {
    // Pega o primeiro processo da lista
    proc = first(high);
    std::cout << "\tHá processo na fila de alta prioridade (Processo "
              << proc->PID << ")" << std::endl;
  }
  // Senão, caso a lista de baixa prioridade possua algum processo
  else if(low.length > 0) {
    // Pega o primeiro processo da lista
    proc = first(low);
    std::cout << "\tNão há processo na fila de alta prioridade; porém, há "
              << "processo na fila de baixa prioridade (Processo "
              << proc->PID << ")" << std::endl;
  }
  // Senão, não executa nenhum processo
  else {
    std::cout << "\tNão há procesos na fila de alta, nem de baixa prioridade"
              << std::endl;
    return -1;
  }

  // Procura o processo na lista de todos os processos para retornar seu index
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    if(proc == all_processes[i]) {
      // Avisa ao usuário
      std::cout << "\tExecutando processo " << proc->PID << std::endl;
      // Modifica o status do processo, e reseta seu tempo antes de preempção
      proc->status = STATUS_RUNNING;
      proc->remaining_time = TIME_SLICE;
      return i;
    }
  }
  // Caso o processo não tenha sido encontrado, temos um problema: o processo
  // está em uma lista de prioridade, mas não está na lista de todos os
  // processos! Sabe??? Zero sentido. Enfim haha
  std::cout << "Existe processo na fila, porém ele não está na lista de todos "
            << "os processos?! Isso não deveria acontecer!" << std::endl;
  return -1;
}

void initializeList(Process**& list) {
  // Inicializa lista preenchida com 0s usando calloc
  list = (Process**)calloc(sizeof(Process*),MAX_PROCESSES);

  // Caso tenha havido um erro na criação
  if(list == NULL) {
    // Avisa para o usuário, e sai do programa
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

// Funções para ajudar na impressão das filas
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
  std::cout << "\t\tDISCO:        " << io_disk.length << " processos" << std::endl;
  std::cout << "\t\t" << p(io_disk, &io_disk_processes) << std::endl;
  std::cout << "\t\tFITA:         " << io_tape.length << " processos" << std::endl;
  std::cout << "\t\t" << p(io_tape, &io_tape_processes) << std::endl;
  std::cout << "\t\tIMPRESSORA:   " << io_printer.length << " processos" << std::endl;
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

    // Caso algum processo esteja sendo executado
    if(current_process_index >= 0) {
      // Executa um ciclo do processo
      executeProcess();
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
    // Caso não haja nenhum processo executando, tenta executar um novo processo
    if(current_process_index < 0)
      current_process_index = tryRunNewProcess();


    sleep_ms(STD_TIMEOUT);

    // Fim do ciclo; imprime o estado atual do sistema
    printSystemState();

    // Confere se ainda existe algum processo inacabado
    if(current_process_index < 0 && !hasUnfinishedProcess())
      // Se não existe, termina o loop
      break;

    sleep_ms(STD_TIMEOUT*3);
  }

  std::cout << "Programa terminado!" << std::endl;
  return 0;
}
