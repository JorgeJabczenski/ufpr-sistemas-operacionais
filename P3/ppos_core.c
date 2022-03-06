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

task_t *scheduler()
{
    debugPrint("inicio sched\n");

#ifdef DEBUG
    printf("tamanho fila antes : %d\n", queue_size((queue_t *)filaTasks));
#endif

    task_t *proxTask = filaTasks;
    queue_remove((queue_t **)&filaTasks, (queue_t *)proxTask);

#ifdef DEBUG
    printf("tamanho fila: %d\n", queue_size((queue_t *)filaTasks));
    printf("proxTask ID: %d\n", proxTask->id);
#endif

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
            if (proximaTask->status != TERMINADA)
            {
                queue_append((queue_t **)&filaTasks, (queue_t *)proximaTask);
            }
        }
    }

    debugPrint("Fim da fila\n");
    task_switch(mainTask);
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
    {   taskAtual->status = TERMINADA;
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
