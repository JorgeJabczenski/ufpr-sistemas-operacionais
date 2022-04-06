// GRR20190372 Jorge Lucas Vicilli Jabczenski

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

#include "ppos.h"
#include "ppos_data.h"

// VARIAVEIS GLOBAIS

int id;
task_t *mainTask;
task_t *taskAtual;
task_t *dispatcher;

task_t *filaTasks;
// task_t *filaTasksSuspensas;

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicialização to timer
struct itimerval timer;

long long int ticksTotais = 0;

//-------------------------------------------------------------------------
// Funcoes Auxiliares

void debugPrint(char *msg)
{
#ifdef DEBUG
    printf("%s", msg);
#endif
}

void *safeMalloc(int size)
{
    void *object = malloc(size);
    if (!object)
    {
        perror("Erro no malloc");
        exit(1);
    }
    return object;
}

void printTask(task_t *task)
{
    printf("Task %d exit: ", task->id);
    printf("execution time %lu ms, ", task->tempoExecucao);
    printf("processor time %lu ms, ", task->tempoDeProcessador);
    printf("activations %lu \n", task->numeroAtivacoes);
}

void print_elem(void *ptr)
{
   task_t *elem = ptr;

   if (!elem)
      return;

   elem->prev ? printf("%d", elem->prev->id) : printf("*");
   printf("<%d>", elem->id);
   elem->next ? printf("%d", elem->next->id) : printf("*");
}

//______________________________________________________________
// Prioridades

void aumentarPrioridadeDinamica(task_t *task)
{
    task->prioridadeDinamica += AGING;

    if (task->prioridadeDinamica < PRIORIDADE_MAXIMA)
        task->prioridadeDinamica = PRIORIDADE_MAXIMA;
}

void task_setprio(task_t *task, int prio)
{
    // Normalizando o valor da prioridade;
    if (prio < PRIORIDADE_MAXIMA)
        prio = PRIORIDADE_MAXIMA;
    else if (prio > PRIORIDADE_MINIMA)
        prio = PRIORIDADE_MINIMA;

    if (task == NULL)
    {
        taskAtual->prioridadeEstatica = prio;
        taskAtual->prioridadeDinamica = prio;
    }
    else
    {
        task->prioridadeEstatica = prio;
        task->prioridadeDinamica = prio;
    }
}

int task_getprio(task_t *task)
{
    if (task == NULL)
        return taskAtual->prioridadeEstatica;

    return task->prioridadeEstatica;
}

//______________________________________________________________ sched e dispatch
task_t *scheduler()
{
    debugPrint("inicio sched\n");

    task_t *proxTask = filaTasks;
    task_t *primeiraTask = filaTasks;

    filaTasks = filaTasks->next;
    while (primeiraTask != filaTasks)
    {
        if (filaTasks->prioridadeDinamica < proxTask->prioridadeDinamica  && proxTask->status == PRONTA)
        {
            // Caso a tarefa nao seja escolhida, aumenta sua prioridade e salva a nova task escolhida
            aumentarPrioridadeDinamica(proxTask);
            proxTask = filaTasks;
        }
        else
        {
            aumentarPrioridadeDinamica(filaTasks);
        }
        filaTasks = filaTasks->next;
    }

    // Retorna a prioridade dinamica da task escolhida para a original
    proxTask->prioridadeDinamica = proxTask->prioridadeEstatica;

    queue_remove((queue_t **)&filaTasks, (queue_t *)proxTask);

    // debugPrint("fim sched\n");
    proxTask->quantidadeTicks = QTD_TICKS;
    proxTask->numeroAtivacoes++;
    return proxTask;
}

void dispatcherBody()
{
    debugPrint("inicio dispatcher\n");

    while (queue_size((queue_t *)filaTasks) > 0)
    {
        #ifdef DEBUG
        queue_print("FILA: ", (queue_t*) filaTasks, print_elem);
        #endif
        taskAtual->numeroAtivacoes++;
        task_t *proximaTask = scheduler();
        if (proximaTask != NULL)
        {
            // printf("TASK ID: %d\n", proximaTask->id);
            task_switch(proximaTask);
            switch (proximaTask->status)
            {
            case PRONTA:
                queue_append((queue_t **)&filaTasks, (queue_t *)proximaTask);
                break;
            case TERMINADA:
                free(proximaTask->context.uc_stack.ss_sp);
                break;
            }
        }
    }

    debugPrint("Fim da fila\n");
    task_exit(0);
}

//------------------------------------------------------------------------- coisas de tempo

// tratador do sinal
void tratador(int signum)
{
    ticksTotais++;
    taskAtual->tempoDeProcessador++;
    if (taskAtual->tarefaUsuario == TRUE)
    {
        taskAtual->quantidadeTicks--;
        if (taskAtual->quantidadeTicks == 0)
            task_yield();
    }
}

void configuraTimer()
{
    action.sa_handler = tratador;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = PRIMEIRO_DISPARO_US; // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = PRIMEIRO_DISPARO_S;   // primeiro disparo, em segundos
    timer.it_interval.tv_usec = DISPARO_US;       // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = DISPARO_S;         // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }
}

unsigned int systime()
{
    return ticksTotais;
}

//------------------------------------------------------------------------- coisas de task
void ppos_init()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);

    debugPrint("Inicio ppos_init\n");

    taskAtual = malloc(sizeof(task_t));
    mainTask = malloc(sizeof(task_t));
    dispatcher = malloc(sizeof(task_t));

    getcontext(&(mainTask->context));
    char *stack = safeMalloc(STACKSIZE);
    mainTask->context.uc_stack.ss_sp = stack;
    mainTask->context.uc_stack.ss_size = STACKSIZE;
    mainTask->context.uc_stack.ss_flags = 0;
    mainTask->context.uc_link = 0;

    mainTask->id = id++;
    mainTask->quantidadeTicks = 0;
    mainTask->status = PRONTA;
    mainTask->tarefaUsuario = TRUE;
    mainTask->prioridadeEstatica = 0;
    mainTask->prioridadeDinamica = 0;
    mainTask->numeroAtivacoes = 0;
    mainTask->tempoExecucao = 0;
    mainTask->tempoInicial = systime();

    queue_append((queue_t **)&filaTasks, (queue_t *)mainTask);

    taskAtual = mainTask;

    task_create(dispatcher, (void *)dispatcherBody, "Dispatcher");
    queue_remove((queue_t **)&filaTasks, (queue_t *)dispatcher);

    // Configura o timer de 1ms
    configuraTimer();

    debugPrint("Fim ppos_init\n");

    task_yield();
}

int task_create(task_t *task, void (*start_routine)(void *), void *arg)
{
    debugPrint("Inicio task_create\n");

    getcontext(&(task->context));

    char *stack = safeMalloc(STACKSIZE);

    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;

    makecontext(&(task->context), (void *)start_routine, 1, arg);

    task->id = id++;
    task->prev = NULL;
    task->next = NULL;
    task->status = PRONTA;
    task->prioridadeEstatica = 0;
    task->prioridadeDinamica = 0;
    task->quantidadeTicks = 0;
    task->numeroAtivacoes = 0;
    task->tempoExecucao = 0;
    task->tempoInicial = systime();

    if (task == dispatcher)
        task->tarefaUsuario = FALSE;
    else
        task->tarefaUsuario = TRUE;

    debugPrint("Adicionado o elemento na fila\n");
    queue_append((queue_t **)&filaTasks, (queue_t *)task);

    debugPrint("Fim task_create\n");
    return task->id;
}

int task_switch(task_t *task)
{
    task_t *taskAux = taskAtual;
    taskAtual = task;
    swapcontext(&(taskAux->context), &(task->context));

    return -1;
}

void task_exit(int exit_code)
{
    taskAtual->tempoFinal = systime();
    taskAtual->tempoExecucao = taskAtual->tempoFinal - taskAtual->tempoInicial;
    taskAtual->status = TERMINADA;
    printTask(taskAtual);
    // task_switch(dispatcher);

    if (taskAtual == dispatcher)
    {
        task_switch(mainTask);
    }
    else
    {
        task_switch(dispatcher);
    }
}

int task_id()
{
#ifdef DEBUG
    printf("task_id: %d\n", taskAtual->id);
#endif

    return taskAtual->id;
}

void task_yield()
{
    task_switch(dispatcher);
}