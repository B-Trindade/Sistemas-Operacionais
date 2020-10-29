#include <ctime>
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


int main() {
  // Determina seed para a função de random
  srand(time(NULL));

  for(int i = 0; i < MAX_PROCESSES; ++i) {
    PIDs[i] = generatePID();
    std::cout << PIDs[i] << std::endl;
  }
  return 0;
}
