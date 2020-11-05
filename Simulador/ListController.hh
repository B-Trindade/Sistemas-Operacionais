#include "Process.hh"
#include "declares.hh"

#ifndef LIST_CONTROLLER_H
#define LIST_CONTROLLER_H

// Controladores das listas
typedef struct ListController {
  int start_index;  // Index do primeiro item da lista
  int length;       // Tamanho de itens da lista
  Process*** list;   // Ponteiro para a lista
} ListController;

// TODO Explicar bonitinho depois
ListController createLC(Process**& list);
// TODO Explicar bonitinho depois
Process* first(ListController& lc);
// TODO Explicar bonitinho depois
void push(ListController& lc, Process* proc);
// TODO Explicar bonitinho depois
Process* shift(ListController& lc);

#endif /* LIST_CONTROLLER_H */