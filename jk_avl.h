#ifndef __JK_AVL_H
#define __JK_AVL_H

#include "jk_types.h"

#define jk_avl_empty_node    ((jk_avl_node_t *)NULL)

typedef struct jk_avl_node_s jk_avl_node_t;

struct jk_avl_node_s {
    jk_uint64_t key;
    void *data;
    short height;
    jk_avl_node_t *left;
    jk_avl_node_t *right;
};


jk_avl_node_t *jk_avl_find_min(jk_avl_node_t *root);
void jk_avl_insert(jk_avl_node_t *new_node, jk_avl_node_t **ptree);
void jk_avl_remove(jk_avl_node_t *node_to_delete, jk_avl_node_t **ptree);


#endif
