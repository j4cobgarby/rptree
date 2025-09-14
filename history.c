#include "history.h"
#include <stdio.h>

hist_event history[HISTORY_SZ] = {{EV_NONE}};
static int hptr = 0;

void push_history(enum hist_type t, pid_t pid, pid_t new_pid, tree *snapshot, char *exec_cmdline) {
  static int counter = 0;

  hist_event *ev = &history[hptr++];
  if (hptr == HISTORY_SZ)
    hptr = 0;

  if (ev->type != EV_NONE) {
    treefree(ev->snapshot);
  }

  ev->seq = counter++;
  ev->type = t;
  ev->pid = pid;
  ev->new_pid = new_pid;
  ev->snapshot = snapshot;
  ev->exec_cmdline = exec_cmdline;
}

void print_history() {
  int pptr = hptr;
  do {
    const hist_event *const ev = &history[--pptr];
    if (pptr == -1)
      pptr = HISTORY_SZ - 1;

    switch (ev->type) {
    case EV_EXIT:
      printf("%3d" C_RED " Exit " C_DIM "%d\n" C_RESET, ev->seq, ev->pid);
      break;
    case EV_FORK:
      printf("%3d" C_GREEN " Fork " C_DIM "%d -> %d\n" C_RESET, ev->seq, ev->pid, ev->new_pid);
      break;
    case EV_EXEC:
      printf("%3d" C_CYAN " Exec " C_DIM "%d, '%s'\n" C_RESET, ev->seq, ev->pid, ev->exec_cmdline);
    case EV_NONE:
      break;
    }
  } while (pptr != hptr);
}
