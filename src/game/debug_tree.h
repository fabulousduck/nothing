#ifndef DEBUG_TREE_H_
#define DEBUG_TREE_H_

typedef struct debug_tree_t debug_tree_t;

debug_tree_t *create_debug_tree(void);
void destroy_debug_tree(debug_tree_t *debug_tree);

#endif  // DEBUG_TREE_H_
