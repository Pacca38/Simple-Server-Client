#include "linked_lists.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct dictionary_node
{
    struct list_node node;
    const char *key;
    size_t key_len;
    void *value;
};

typedef struct dictionary
{
    struct dictionary_node **nodes;
    size_t hashmap_size;
} dict_t;

size_t djb33x_hash(const char *key, const size_t keylen);

struct dictionary *dict_increase_hashmap_size(dict_t *dict, const size_t new_hashmap_size);

size_t get_index_from_key(const char *key, size_t key_len, size_t hashmap_size);

void dict_print(dict_t *dict);

int dict_get_count(dict_t *table);

struct dictionary *dict_new(const size_t hashmap_size);

struct dictionary_node *dict_search(dict_t *table, const char *key, const size_t key_len);

struct dictionary_node *dict_get_from_index(dict_t *table, int index);

struct dictionary_node *dict_insert(dict_t **table, const char *key, const size_t key_len, void *value);

struct dictionary_node *dict_remove(dict_t *table, const char *key, const size_t key_len);

struct dictionary *dict_increase_hashmap_size(dict_t *dict, const size_t new_hashmap_size);

void dict_destroy(dict_t *dict);