#include <ctime>
#include <time.h>
#include <iostream>

#define MAX_PROCESSES 10

// Inicializa o array de PIDs com 0s
int PIDs[MAX_PROCESSES] = {};

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
      if(PIDs[i] == pid) {
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


int main() {
  // Determina seed para a função de random
  srand(time(NULL));

  std::cout << "Programa de exemplo:" << std::endl;
  std::cout << "\tGerando PIDs distintos e adicionado ao array" << std::endl;
  std::cout << "\tcom delay de 200ms entre cada adição" << std::endl;
  for(int i = 0; i < MAX_PROCESSES; ++i) {
    sleep_ms(200);
    PIDs[i] = generatePID();

    std::cout << "\t[ ";
    for(int j = 0; j < MAX_PROCESSES; ++j) {
      std::cout << PIDs[j] << " ";
    }
    std::cout << ']' << std::endl;
  }
  return 0;
}
