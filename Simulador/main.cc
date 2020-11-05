#include <ctime>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <iterator>

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

ListController high = createLC(high_processes);
ListController low = createLC(low_processes);

// Quando são colocados dentro da função `initializeProcesses()`, essas
// variáveis são descartadas e removidas da memória; assim, o Process fica com
// lixo no lugar :(
IO_Operation IO_Limiter = createIO(-1,-1);
IO_Operation a[2] = { createIO(IO_IMPRESSORA,2), IO_Limiter };
IO_Operation b[3] = { createIO(IO_IMPRESSORA,4), createIO(IO_FITA,7), IO_Limiter };
IO_Operation c[4] = { createIO(IO_DISCO,1), createIO(IO_FITA,2), createIO(IO_IMPRESSORA,6),IO_Limiter };

// Inicializa processos
void initializeProcesses() {
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    int t = i % 4;
    int random_total_time = (std::rand() % 5)+2;
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

void executeProcess() {
  // TODO: conferir se está em IO etc
  Process* proc = all_processes[current_process_index];
  proc->remaining_time--;
  proc->total_time--;
  std::cout << "\tExecutando processo " << proc->PID << std::endl;
  std::cout << "\tFalta " << (proc->total_time) << " u.t." << std::endl;
  if(proc->total_time <= 0) {
    std::cout << "\tProcesso terminado!" << std::endl;
    proc->status = STATUS_TERMINATED;
    current_process_index = -1;
    // Remove processo da lista em que ele está
    if(proc->priority == PRIORITY_HIGH)
      shift(high);
    else
      shift(low);
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

int tryRunNewProcess() {
  std::cout << "\tNão há processos executando. Procurando um processo com estado "
            << "'READY' para ser executado." << std::endl;
  Process* proc;
  if(high.length > 0) {
    proc = first(high);
  } else if(low.length > 0) {
    proc = first(low);
  } else {
    std::cout << "\tNenhum processo encontrado!" << std::endl;
    return -1;
  }
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    if(proc == all_processes[i]) {
      std::cout << "\tEncontrado! Executando processo " << proc->PID << std::endl;
      proc->status = STATUS_RUNNING;
      proc->remaining_time = TIME_SLICE;
      return i;
    }
  }
  std::cout << "WOOPS EM BREVE: TELA AZUL c:" << std::endl;
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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main() {
  // Determina seed para a função de random
  srand(time(NULL));

  // Cria as listas de processos
  initializeList(all_processes);
  initializeList(high_processes);
  initializeList(low_processes);

  // Inicializa os processos (hardcoded)
  initializeProcesses();

  sleep_ms(500);
  std::cout << std::endl;
  std::cout << "Todos os processos criados! Iniciando processador..." << std::endl;
  std::cout << std::endl;
  sleep_ms(1500);

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

    sleep_ms(500);

    // Confere se o processo sendo executado deve sofrer preempção
    checkForPreemption();

    sleep_ms(500);

    // Confere se existe algum processo que inicia nesse ciclo
    // (e o coloca na fila apropriada)
    checkForNewProcess(global_cycle_count);

    sleep_ms(500);

    // Confere se algum processo acabou de terminar operação de I/O
    // (e o coloca na fila apropriada)
    checkForFinishedIO(global_cycle_count);

    sleep_ms(500);

    // (TODO: conferir essa ordem de prioridades)
    if(current_process_index < 0)
      current_process_index = tryRunNewProcess();

    sleep_ms(1500);
  }

  std::cout << "Programa terminado!" << std::endl;
  return 0;
}
