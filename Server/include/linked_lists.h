#include <stddef.h>

struct list_node
{
    struct list_node *next;
};

int list_get_length(struct list_node **head);

struct list_node *list_get_tail(struct list_node **head);

struct list_node *list_append(struct list_node **head, struct list_node *item);

struct list_node *list_pop(struct list_node **head);

struct list_node *list_find(struct list_node **head, struct list_node *item);

struct list_node *list_get(struct list_node **head, int index);

struct list_node *list_remove(struct list_node **head, struct list_node *item);

struct list_node *list_reverse(struct list_node **head);