#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue)
{
    int tamanho;
    queue_t *primeiro = queue;

    if (queue == NULL)
        return 0;

    tamanho = 1;
    while (queue->next != primeiro)
    {
        tamanho++;
        queue = queue->next;
    }
    return tamanho;
}

int queue_append(queue_t **queue, queue_t *elem)
{
    if (elem == NULL || elem->prev != NULL || elem->next != NULL)
        return -1;

    if (*queue == NULL)
    {
        (*queue) = elem;
        (*queue)->prev = elem;
        (*queue)->next = elem;
    }
    else
    {
        elem->prev = (*queue)->prev;
        elem->next = (*queue);
        (*queue)->prev->next = elem;
        (*queue)->prev = elem;
    }

    return 0;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    if (elem == NULL)
        return -1;

    queue_t *first = (*queue);

    if (queue_size(*queue) == 1)
    {
        (*queue)->prev = NULL;
        (*queue)->next = NULL;
        (*queue) = NULL;
    }
    else
    {
        do
        {
            if ((*queue) == (elem))
            {
                (*queue)->prev->next = (*queue)->next;
                (*queue)->next->prev = (*queue)->prev;

                if ((*queue) == first)
                {
                    (*queue) = (*queue)->next;
                } else {
                    (*queue) = (first);
                }

                elem->next = NULL;
                elem->prev = NULL;
                return 0;
            }
            else
            {
                (*queue) = (*queue)->next;
            }
        } while ((*queue) != first);
    }
    return 0;
}