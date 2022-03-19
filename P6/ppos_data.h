// GRR20190372 Jorge Lucas Vicilli Jabczenski
// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"

#define STACKSIZE 64 * 1024 /* tamanho de pilha das threads */

#define PRONTA    0
#define TERMINADA 1
#define SUSPENSA  2

#define AGING    -1
#define PRIORIDADE_MINIMA  20
#define PRIORIDADE_MAXIMA -20

#define PRIMEIRO_DISPARO_US 1000   // primeiro disparo, em micro-segundos
#define PRIMEIRO_DISPARO_S     0   // primeiro disparo, em segundos
#define DISPARO_US          1000   // disparos subsequentes, em micro-segundos
#define DISPARO_S              0   // disparos subsequentes, em segundos

#define QTD_TICKS  20

#define SIM 1
#define NAO 0


// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;			                	// identificador da tarefa
  ucontext_t context ;			      // contexto armazenado da tarefa
  short status ;			            // pronta, rodando, suspensa, ...
  short preemptable ;			        // pode ser preemptada?
  int prioridadeEstatica;         // prioridade da task
  int prioridadeDinamica;         // prioridade da task
  int quantidadeTicks;            // quantidade de ciclos que uma tarefa tem
  int tarefaUsuario;              // flag para indicar se é uma tarefa de usuario
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

