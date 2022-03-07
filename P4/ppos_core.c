// GRR20190372 Jorge Lucas Vicilli Jabczenski

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "ppos.h"
#include "ppos_data.h"

int id;
task_t *mainTask;
task_t *taskAtual;
task_t *dispatcher;

task_t *filaTasks;

//-------------------------------------------------------------------------
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
//______________________________________________________________

void aumentarPrioridadeDinamica(task_t *task)
{
    task->prioridadeDinamica += AGING;
    if (task->prioridadeDinamica < PRIORIDADE_MAXIMA)
    {
        task->prioridadeDinamica = PRIORIDADE_MAXIMA;
    }
}

void task_setprio(task_t *task, int prio)
{
    // Normalizando o valor da prioridade;
    if (prio < -20)
        prio = -20;
    else if (prio > 20)
        prio = 20;

    task->prioridadeEstatica = prio;
    task->prioridadeDinamica = prio;
}

int task_getprio(task_t *task)
{
    if (task == NULL)
    {
        return taskAtual->prioridadeEstatica;
    }
    return task->prioridadeEstatica;
}

task_t *scheduler()
{
    debugPrint("inicio sched\n");

    task_t *proxTask = filaTasks;
    task_t *primeiraTask = filaTasks;

    filaTasks = filaTasks->next;
    while (primeiraTask != filaTasks)
    {
        if (filaTasks->prioridadeDinamica <= proxTask->prioridadeDinamica)
        {
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

    debugPrint("fim sched\n");
    return proxTask;
}

void dispatcherBody()
{
    debugPrint("inicio dispatcher\n");

    while (queue_size((queue_t *)filaTasks) > 0)
    {
        task_t *proximaTask = scheduler();
        if (proximaTask != NULL)
        {
            task_switch(proximaTask);
            switch (proximaTask->status)
            {
            case PRONTA:
                queue_append((queue_t **)&filaTasks, (queue_t *)proximaTask);
                break;
            }
        }
    }

    debugPrint("Fim da fila\n");
    task_exit(0);
}

void task_yield()
{
    task_switch(dispatcher);
}

//-------------------------------------------------------------------------
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
    mainTask->prev = NULL;
    mainTask->next = NULL;

    taskAtual = mainTask;

    task_create(dispatcher, (void *)dispatcherBody, "Dispatcher");
    queue_remove((queue_t **)&filaTasks, (queue_t *)dispatcher);

    debugPrint("Fim ppos_init\n");
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

#ifdef DEBUG
    printf("task_create: %d\n", task->id);
#endif

    debugPrint("Adicionado o elemento na fila\n");
    queue_append((queue_t **)&filaTasks, (queue_t *)task);

    debugPrint("Fim task_create\n");
    return task->id;
}

int task_switch(task_t *task)
{

#ifdef DEBUG
    printf("task_switch: trocando contexto %d -> %d\n", taskAtual->id, task->id);
#endif

    task_t *taskAux = taskAtual;
    taskAtual = task;
    swapcontext(&(taskAux->context), &(task->context));

    return -1;
}

void task_exit(int exit_code)
{
    if (taskAtual == dispatcher)
    {
        task_switch(mainTask);
    }
    else
    {
        taskAtual->status = TERMINADA;
        task_t *taskAux = taskAtual;
        task_switch(dispatcher);
        free(taskAux->context.uc_stack.ss_sp);
    }
}

int task_id()
{
#ifdef DEBUG
    printf("task_id: %d\n", taskAtual->id);
#endif

    return taskAtual->id;
}
