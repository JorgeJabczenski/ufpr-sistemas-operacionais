#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue)
{
    int tamanho;
    queue_t *primeiro = queue;

    // Retorno 0 caso a lista esteja vazia
    if (queue == NULL)
        return 0;

    // Caminha pela lista até encontrar o mesmo elemento
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
    if (queue == NULL)
    {
        perror("Fila Inexistente");
        return -1;
    }

    if (elem == NULL)
    {
        perror("Elemento Inexistente");
        return -2;
    }

    if (elem->prev != NULL || elem->next != NULL)
    {
        perror("Elemento já existe em outra fila");
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
    int tamanho = queue_size(queue);
    printf("%s: [", name);

    for (int i = 0; i < tamanho; i++)
    {
        print_elem((void *)queue);
        queue = queue->next;
        (i < tamanho - 1) ? printf(" ") : i;
    }
    printf("]\n");
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    if (queue == NULL)
    {
        perror("Fila Inexistente");
        return -1;
    }

    if (elem == NULL)
    {
        perror("Elemento Inexistente");
        return -2;
    }

    if (elem->next == NULL || elem->prev == NULL)
    {
        perror("Elemento não pertence a nenhuma fila");
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
    }

    // O elemento não pertencia a aquela fila
    perror("Elemento não pertencia a fila indicada");
    return -3;

}