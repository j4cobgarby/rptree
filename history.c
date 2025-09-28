#include "history.h"
#include <stdio.h>

hist_event history[HISTORY_SZ] = {{EV_NONE}};
static int hptr = 0;

void push_history(enum hist_type t, pid_t pid, pid_t new_pid, tree *snapshot,
                  char *exec_cmdline) {
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
  int n, haslines = 0;

  printf(C_BGWHITE C_BLACK);
  n = printf("   # Type     PID  Details");
  printf("%*s\n", win_cols() - n, "");
  printf(C_RESET);

  do {
    if (--pptr == -1)
      pptr = HISTORY_SZ - 1;
    const hist_event *const ev = &history[pptr];

    if (ev->type != EV_NONE) {
      haslines = 1; // There are some history lines

      switch (ev->type) {
      case EV_EXIT:
        printf("%4d" C_RED " EXIT %7d \n" C_RESET, ev->seq, ev->pid);
        break;
      case EV_FORK:
        printf("%4d" C_GREEN " FORK %7d -> %d\n" C_RESET, ev->seq, ev->pid,
               ev->new_pid);
        break;
      case EV_EXEC:
        printf("%4d" C_BLUE " EXEC %7d '%s'\n" C_RESET, ev->seq, ev->pid,
               ev->exec_cmdline);
        break;
      default:
        break;
      }
    } else {
      break;
    }
  } while (pptr != hptr);

  if (!haslines) printf("No events yet....\n");
}
