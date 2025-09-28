#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

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

int win_cols() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  return w.ws_col;
}

void print_proc_cmdline(pid_t pid) {
  char *cmdline = get_cmdline(pid);
  printf("%s", cmdline);
  free(cmdline);
}

void get_stdfds(pid_t pid, char **links, const size_t bufsz) {
  char fnbuf[48];

  for (int i = 0; i < 3; i++) {
    snprintf(fnbuf, sizeof(fnbuf), "/proc/%d/fd/%d", pid, i);

    if (readlink(fnbuf, links[i], bufsz) == -1) {
      strncpy(links[i], "<error>", bufsz);
      continue;
    }
  }
}

int important_file(const char *fname) {
  return strncmp(fname, "pipe:", 5) == 0;
}

struct proc_stat {
  pid_t pid;
  char state;
  pid_t ppid;
  pid_t pgrp;
  int session;
  int tty_nr;
  pid_t tpgid;
};

struct proc_stat read_stat(pid_t pid) {
  char fnbuf[48];
  struct proc_stat ret = {0};
  snprintf(fnbuf, sizeof(fnbuf), "/proc/%d/stat", pid);
  FILE *f = fopen(fnbuf, "r");
  if (!f)
    return ret;

  fscanf(f, "%d %*s %c %d %d %d %d %d", &ret.pid, &ret.state, &ret.ppid,
         &ret.pgrp, &ret.session, &ret.tty_nr, &ret.tpgid);

  return ret;
}

#define FILE_COLOUR(fname) (important_file(fname) ? C_CYAN : C_DIM)

int col_widths[] = {7, 16, 16, 16, 7, 7, 1};

const char *col_titles[] = {"PID",   "stdin", "stdout",  "stderr", "pgrp",
                            "tpgid", "state"};

void print_pid_info(pid_t pid, int colspec) {
  char **fdlinks = malloc(sizeof(char *) * 3);
  for (int i = 0; i < 3; i++) {
    fdlinks[i] = malloc(17);
  }
  get_stdfds(pid, fdlinks, 16);
  struct proc_stat stat = read_stat(pid);

  if ((colspec >> COL_PID) & 1) {
    printf(C_YELLOW "%*d " C_RESET, col_widths[COL_PID], pid);
  }
  if ((colspec >> COL_STDIN) & 1) {
    printf("%s%*.*s " C_RESET, FILE_COLOUR(fdlinks[STDIN_FILENO]),
           col_widths[COL_STDIN], col_widths[COL_STDIN], fdlinks[STDIN_FILENO]);
  }
  if ((colspec >> COL_STDOUT) & 1) {
    printf("%s%*.*s " C_RESET, FILE_COLOUR(fdlinks[STDOUT_FILENO]),
           col_widths[COL_STDOUT], col_widths[COL_STDOUT],
           fdlinks[STDOUT_FILENO]);
  }
  if ((colspec >> COL_STDERR) & 1) {
    printf("%s%*.*s " C_RESET, FILE_COLOUR(fdlinks[STDERR_FILENO]),
           col_widths[COL_STDERR], col_widths[COL_STDERR],
           fdlinks[STDERR_FILENO]);
  }
  if ((colspec >> COL_PGRP) & 1) {
    printf(C_MAGENTA "%*d " C_RESET, col_widths[COL_PGRP], stat.pgrp);
  }
  if ((colspec >> COL_TPGID) & 1) {
    printf(C_GREEN "%*d " C_RESET, col_widths[COL_TPGID], stat.tpgid);
  }
  if ((colspec >> COL_STATE) & 1) {
    printf("%*c ", col_widths[COL_STATE], stat.state);
  }

  for (int i = 0; i < 3; i++) {
    free(fdlinks[i]);
  }
  free(fdlinks);
}

void treeprint_impl(tree *root, const char *pref, int edge, int colspec) {
  if (!root)
    return;

  print_pid_info(root->pid, colspec);
  if (edge == -1) {
    printf("%s", pref);
  } else {
    printf(" %s%s", pref, edge ? "└─ " : "├─ ");
  }

  // Print process's cmdline args
  printf(C_BLUE);
  print_proc_cmdline(root->pid);
  printf(C_RESET);

  printf("\n");

  char new_pref[256];
  snprintf(new_pref, sizeof(new_pref), "%s%s", pref,
           edge == -1 ? ""
           : edge     ? "   "
                      : "|  ");

  LL_FOREACH(&root->children, el) {
    tree *subtree = el->data;
    treeprint_impl(subtree, new_pref, !el->next, colspec);
  }
}

void treeprint(tree *root, int colspec) {
  printf(C_BGWHITE C_BLACK);
  int done_width = 0;
  for (int i = 0; i < N_COL_TYPES; i++) {
    if ((colspec >> i) & 1) {
      printf("%*.*s ", col_widths[i], col_widths[i], col_titles[i]);
      done_width += col_widths[i] + 1;
    }
  }
  printf("%*s", win_cols() - done_width, "");
  printf("\n" C_RESET);

  treeprint_impl(root, "", -1, colspec);
}

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
