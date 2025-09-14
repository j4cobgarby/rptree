#ifndef __INCLUDE_HISTORY_H__
#define __INCLUDE_HISTORY_H__

#define HISTORY_SZ 25

#include <unistd.h>

#include "utils.h"

enum hist_type {
  EV_NONE,
  EV_EXIT,
  EV_FORK,
  EV_EXEC,
};

typedef struct hist_event {
  int seq;
  enum hist_type type;

  pid_t pid;
  pid_t new_pid;

  tree *snapshot;
  char *exec_cmdline;
} hist_event;

extern hist_event history[HISTORY_SZ];

void push_history(enum hist_type t, pid_t pid, pid_t new_pid, tree *snapshot,
                  char *exec_cmdline);
void print_history();

#endif
