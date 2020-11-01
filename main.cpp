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

typedef struct IO_Operation {
  int type;             // Identificador do IO
  int start_time;       // Momento de início do IO
} IO_Operation;


// Inicializa o array de PIDs com 0s
int PIDs[MAX_PROCESSES] = {};

class Process
{
public:
	int PID;				// Identificador do processo
	int PPID;				// Identificador do processo pai (sempre = 1)
	int status;				// Status do processo
	int total_time;			// Tempo total a ser executado
	int elapsed_time;		// Tempo já executado
	int priority;			// Nível de prioridade do processo
	IO_Operation* IOs;		// Array de momentos de I/O

private:
	// Variaveis membro
	const int m_MAX_ATTEMPTS = 100;
	int m_tries = 0;
	bool m_is_new_pid = false;
	int m_pid = 0;

public:
	//Construtores com e sem passagem dos parametros total_time & ios
	Process() : PID( generatePID() ), status(STATUS_NEW), priority(PRIORITY_HIGH) {}

	Process( int total_time, IO_Operation* ios) : PID( generatePID() ), status(STATUS_NEW),
	 priority(PRIORITY_HIGH), total_time(total_time), IOs(ios), elapsed_time(0) {}
	
private:

	//Gera PID novo, exclusivo para o processo
	int generatePID() {
		while(!m_is_new_pid) {

			// Gera PID entre (1,99]
			m_pid = (std::rand() % 98)+2;

			// Confere se o PID já existe
			m_is_new_pid = true;
			for(int i = 0; i < MAX_PROCESSES; ++i) {
				if(PIDs[i] == m_pid) {			// Se o PID já está presente na lista, precisamos gerar outro
					m_is_new_pid = false;				// Sai do `for` e garante outro loop do `while` externo
					break;			
				}
			}

			// Incrementa número de tentativas de gerar um PID novo
			m_tries++;
			// Se as tentativas ultrapassarem o limite, retorna valor de -1
			if(m_tries > m_MAX_ATTEMPTS)
				return -1;
		}
		return m_pid;
	}

};


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

int main() {
  // Determina seed para a função de random
  srand(time(NULL));

  IO_Operation test_ios[3] = {
    createIO(IO_FITA,2),
    createIO(IO_DISCO,1),
    createIO(IO_IMPRESSORA,6)
  };

  Process p(10, test_ios);

  std::cout << "Processo de PID " << p.PID << std::endl;
  std::cout << "\tTotal time: " << p.total_time << " u.t." << std::endl;
  std::cout << "\tElapsed time: " << p.elapsed_time << " u.t." << std::endl;

  // sleep_ms(200);
  return 0;
}
