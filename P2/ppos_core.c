// GRR20190372 Jorge Lucas Vicilli Jabczenski

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "ppos.h"
#include "ppos_data.h"

int id;
task_t *mainTask;
task_t *taskAtual;
ucontext_t mainContext;
ucontext_t taskAtualContext;

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

void ppos_init()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);

    debugPrint("Inicio ppos_init\n");

    taskAtual = malloc(sizeof(task_t));
    mainTask = malloc(sizeof(task_t));

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

#ifdef DEBUG
    printf("task_create: %d\n", task->id);
#endif
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
    task_t *taskAux = taskAtual;
    task_switch(mainTask);
    free(taskAux->context.uc_stack.ss_sp);
}

int task_id()
{
#ifdef DEBUG
    printf("task_id: %d\n", taskAtual.id);
#endif

    return taskAtual->id;
}
