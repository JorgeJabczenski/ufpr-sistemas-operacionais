// GRR20190372 Jorge Lucas Vicilli Jabczenski

#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue)
{
    int size;
    queue_t *first = queue;

    // Retorno 0 caso a lista esteja vazia
    if (queue == NULL)
        return 0;

    // Caminha pela lista até encontrar o mesmo elemento
    size = 1;
    while (queue->next != first)
    {
        size++;
        queue = queue->next;
    }
    return size;
}

int queue_append(queue_t **queue, queue_t *elem)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Fila Inexistente\n");
        return -1;
    }

    if (elem == NULL)
    {
        fprintf(stderr, "Elemento Inexistente\n");
        return -2;
    }

    if (elem->prev != NULL || elem->next != NULL)
    {
        fprintf(stderr, "Elemento já existe em outra fila\n");
        return -3;
    }

    if (*queue == NULL) // Caso a fila não possua nenhum elemento
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
    int size = queue_size(queue);
    printf("%s: [", name);

    for (int i = 0; i < size; i++)
    {
        print_elem((void *)queue);
        queue = queue->next;
        (i < size - 1) ? printf(" ") : i;
    }
    printf("]\n");
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    if (queue == NULL)
    {
        fprintf(stderr, "Fila Inexistente\n");
        return -1;
    }

    if (elem == NULL)
    {
        fprintf(stderr, "Elemento Inexistente\n");
        return -2;
    }

    if (elem->next == NULL || elem->prev == NULL)
    {
        fprintf(stderr, "Elemento não pertence a nenhuma fila\n");
        return -3;
    }

    queue_t *first = (*queue);

    if (queue_size(*queue) == 1 && (*queue) == (elem))
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
                }
                else
                {
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

        // O elemento não pertencia a aquela fila
        fprintf(stderr, "Elemento não pertencia a fila indicada\n");
        return -3;
    }
    return 0;
}