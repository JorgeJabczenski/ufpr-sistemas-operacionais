#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ppos.h"
#include "queue.h"

#define N 5

semaphore_t sem_item;   // itens
semaphore_t sem_buffer; // mutex
semaphore_t sem_vagas;  // quantos podem ler do buffer

queue_t *buffer;

task_t prod1, prod2, prod3, cons1, cons2;

typedef struct item_t
{
    struct item_t *prev, *next;
    int id;
} item_t;

void produtor(void *arg)
{
    while (1)
    {
        task_sleep(1000);
        item_t *item = malloc(sizeof(item_t));
        item->id = random() % 87;
        item->prev = NULL;
        item->next = NULL;

        sem_down(&sem_item);
        sem_down(&sem_buffer);
        queue_append(&buffer, (item_t *) item);
        sem_up(&sem_buffer);

        sem_up(&sem_vagas);

        printf("%s produziu %d\n", (char *)arg, item->id);
    }
}

void consumidor(void *arg)
{
    while (1)
    {

        item_t *item;

        sem_down(&sem_item);
        sem_down(&sem_buffer);
        item = (item_t *)buffer;
        queue_remove(&item, buffer);
        sem_up(&sem_buffer);

        sem_up(&sem_vagas);

        printf("%s consumiu %d\n", (char *)arg, item->id);
        free(item);
        task_sleep(1000);
    }
}

int main(){
    sem_create(&sem_item, 0);
    sem_create(&sem_vagas, N);
    sem_create(&sem_buffer, 1);

    ppos_init();
    //cria tasks
    task_create (&prod1, produtor, "p1") ;
    task_create (&prod2, produtor, "p2") ;
    task_create (&prod3, produtor, "p3") ;
    task_create (&cons1, consumidor, "                          c1");
    task_create (&cons2, consumidor, "                          c2");
    
    task_join(&prod1);
    task_exit(0);
    return 0;
}