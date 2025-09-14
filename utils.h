#ifndef __INCLUDE_UTILS_H__
#define __INCLUDE_UTILS_H__

#include <unistd.h>

#define C_CYAN "\033[36m"
#define C_YELLOW "\033[33m"
#define C_GREEN "\033[32m"
#define C_RED "\033[31m"
#define C_RESET "\033[0m"
#define C_DIM "\033[2m"

struct ll_node {
  void *data;
  struct ll_node *next, *prev;
};

typedef struct ll_node *ll_list;

typedef struct tree {
  pid_t pid;
  ll_list children;
} tree;

void *ll_at(ll_list *list, int i);
void ll_push_back(ll_list *list, void *data);
void ll_push_front(ll_list *list, void *data);
void *ll_pop_back(ll_list *list);
void *ll_pop_front(ll_list *list);
unsigned long ll_len(ll_list *list);

tree *mksubtree(pid_t pid);
void treeprint_impl(tree *root, const char *pref, int edge);
void treeprint(tree *root);
tree *treefind(tree *root, pid_t pid);
void treefree(tree *root);
void treedel(tree *root, pid_t pid);
void treeadd(tree *root, pid_t parent, pid_t child);

void print_proc_cmdline(pid_t pid);
char *get_cmdline(pid_t pid);

#define LL_FOREACH(list, el) for (struct ll_node *el = *list; el; el = el->next)

// list: An ll_list* type.
// result: A pointer type, which will get set to either NULL or the found data
// pred: An expression which can use `data`, which is cast to be the same type
// as result, returning true if found
#define LL_FIND(list, result, pred)                                            \
  {                                                                            \
    auto m = *list;                                                            \
    result = NULL;                                                             \
    for (; m; m = m->next) {                                                   \
      typeof(result) data = (typeof(result))m->data;                           \
      if (pred) {                                                              \
        result = data;                                                         \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

#define LL_FIND_POP(list, result, pred)                                        \
  {                                                                            \
    result = NULL;                                                             \
    for (struct ll_node *m = *list; m; m = m->next) {                          \
      typeof(result) data = (typeof(result))m->data;                           \
      if (pred) {                                                              \
        if (m->prev) {                                                         \
          m->prev->next = m->next;                                             \
        } else {                                                               \
          *list = m->next;                                                     \
        }                                                                      \
                                                                               \
        if (m->next) {                                                         \
          m->next->prev = m->prev;                                             \
        }                                                                      \
                                                                               \
        result = data;                                                         \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

#define LL_DELETE_SOME(list, pred, cleanup)                                    \
  {                                                                            \
    auto m = *list;                                                            \
    while (m) {                                                                \
      void *data = (void *)m->data;                                            \
      if (pred) {                                                              \
        if (m->prev) {                                                         \
          m->prev->next = m->next;                                             \
        } else {                                                               \
          *list = m->next;                                                     \
        }                                                                      \
                                                                               \
        if (m->next) {                                                         \
          m->next->prev = m->prev;                                             \
        }                                                                      \
                                                                               \
        cleanup;                                                               \
        struct ll_node *next = m->next;                                        \
        free(m);                                                               \
        m = next;                                                              \
      } else {                                                                 \
        m = m->next;                                                           \
      }                                                                        \
    }                                                                          \
  }

#endif
