#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct ll_node *mknode(void *data) {
  struct ll_node *node = malloc(sizeof(struct ll_node));
  if (!node) {
    fprintf(stderr, "Out of memory!\n");
    exit(-1);
  }
  node->next = NULL;
  node->prev = NULL;
  node->data = data;
  return node;
}

void *ll_at(ll_list *list, int i) {
  struct ll_node *m = *list;
  for (int j = 0; j < i; j++) {
    if (!m)
      break;
    m = m->next;
  }
  return m ? m->data : NULL;
}

void ll_push_back(ll_list *list, void *data) {
  struct ll_node *n = mknode(data);

  if (*list == NULL) {
    *list = n;
  } else {
    struct ll_node *m = *list;
    for (; m->next; m = m->next)
      ;
    m->next = n;
    n->prev = m;
  }
}

void ll_push_front(ll_list *list, void *data) {
  struct ll_node *n = mknode(data);

  if (*list == NULL) {
    *list = n;
  } else {
    n->next = *list;
    (*list)->prev = n;
    *list = n;
  }
}

void *ll_pop_back(ll_list *list) {
  if (*list == NULL) {
    return NULL;
  } else {
    struct ll_node *m = *list;
    for (; m->next; m = m->next)
      ;
    void *ret = m->data;
    if (m->prev)
      m->prev->next = NULL;
    else
      *list = NULL;
    free(m);
    return ret;
  }
}

void *ll_pop_front(ll_list *list) {
  if (*list == NULL) {
    return NULL;
  } else {
    struct ll_node *m = *list;
    *list = (*list)->next;
    void *ret = m->data;
    if (m->next)
      m->next->prev = NULL;
    else
      *list = NULL;
    free(m);
    return ret;
  }
}

unsigned long ll_len(ll_list *list) {
  size_t i = 0;
  for (struct ll_node *m = *list; m; m = m->next, ++i)
    ;
  return i;
}

tree *mksubtree(pid_t pid) {
  tree *t = malloc(sizeof(tree));
  if (!t) {
    perror("malloc");
    exit(-1);
  }
  t->pid = pid;
  t->children = NULL;
  return t;
}

char *get_cmdline(pid_t pid) {
  char fnbuf[48];
  snprintf(fnbuf, sizeof(fnbuf), "/proc/%d/cmdline", pid);

  FILE *cf = fopen(fnbuf, "rb");
  if (!cf) {
    return NULL;
  }

  char *ret = malloc(128); // Cap cmdline read to 127 + \0
  int retind = 0;          // Cursor in 'ret'

  do {
    int nread = fread(ret + retind, 1, 127 - retind, cf);
    retind += nread;
    if (nread == 0)
      break;
  } while (1);

  for (int ci = 0; ci < retind; ci++) {
    if (ret[ci] == '\0')
      ret[ci] = ' ';
  }

  ret[retind - 1] = '\0';
  fclose(cf);

  return ret;
}

void print_proc_cmdline(pid_t pid) {
  char *cmdline = get_cmdline(pid);
  printf("%s", cmdline);
  free(cmdline);
}

void print_pid_info(pid_t pid) {
  printf(C_YELLOW "%d" C_RESET, pid);

  printf(" " C_CYAN);
  print_proc_cmdline(pid);
  printf(C_RESET);
}

void treeprint_impl(tree *root, const char *pref, int edge) {
  if (!root)
    return;

  printf("%s%s", pref, edge ? "└─ " : "├─ ");
  print_pid_info(root->pid);
  printf("\n");

  char new_pref[256];
  snprintf(new_pref, sizeof(new_pref), "%s%s", pref, edge ? "   " : "|  ");

  LL_FOREACH(&root->children, el) {
    tree *subtree = el->data;
    treeprint_impl(subtree, new_pref, !el->next);
  }
}

void treeprint(tree *root) { treeprint_impl(root, "", 1); }

tree *treefind(tree *root, pid_t pid) {
  if (!root)
    return NULL;
  if (root->pid == pid) {
    return root;
  }
  LL_FOREACH(&root->children, el) {
    tree *subtree = el->data;
    tree *found = treefind(subtree, pid);
    if (found) {
      return found;
    }
  }
  return NULL;
}

void treefree(tree *root) {
  if (!root)
    return;
  LL_FOREACH(&root->children, el) {
    tree *subtree = el->data;
    treefree(subtree);
  }
  free(root);
}

void treedel(tree *root, pid_t pid) {
  if (!root)
    return;
  tree *deleted;

  LL_FIND_POP(&root->children, deleted, data->pid == pid);
  if (deleted) {
    treefree(deleted);
  }

  LL_FOREACH(&root->children, el) {
    tree *subtree = el->data;
    treedel(subtree, pid);
  }
}

void treeadd(tree *root, pid_t parent, pid_t child) {
  if (!root)
    return;
  tree *subtree = treefind(root, parent);
  if (!subtree) {
    printf("Couldn't find parent with PID=%d\n", parent);
    return;
  }
  ll_push_back(&subtree->children, mksubtree(child));
}
