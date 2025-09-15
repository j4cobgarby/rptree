#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "utils.h"

void tracer(pid_t);

void screenfull(tree *root) {
  printf("\033[2J\033[H"); // Clear terminal and jump to top
  printf("Recent Events:\n");
  print_history();
  printf("\nProcess Tree:\n");
  treeprint(root);
}

void usage(const char *exe) {
  fprintf(stderr, "Usage:\t%s PID\n\t%s -s ARGS...\n", exe, exe);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    usage(argv[0]);
  }

  pid_t pid;

  if (strcmp(argv[1], "-s") == 0) {
    fprintf(stderr, "Not implemented yet...\n");
    exit(EXIT_FAILURE);
    // printf("Executing '%s'\n", argv[2]);
    // pid = fork();
    // if (pid == 0) {
    //   if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
    //     perror("ptrace TRACEME");
    //     exit(EXIT_FAILURE);
    //   }
    //   if (execvp(argv[2], &argv[2]) == -1) {
    //     perror("execvp");
    //     exit(EXIT_FAILURE);
    //   }
    // }
    // printf("Will trace child, pid = %d\n", pid);
  } else {
    if (argc > 2) {
      fprintf(stderr, "Too many arguments.\n");
      usage(argv[0]);
    }

    errno = 0;
    char *endptr;
    pid = strtoul(argv[1], &endptr, 10);
    if (errno || endptr == argv[1]) {
      fprintf(stderr, "Invalid PID: '%s'\n", argv[1]);
      usage(argv[0]);
    }
  }

  tracer(pid);

  return 0;
}

void initialise_tree(tree *root) {
  char fnbuf[48];
  snprintf(fnbuf, sizeof(fnbuf), "/proc/%d/task/%d/children", root->pid,
           root->pid);
  FILE *cf = fopen(fnbuf, "r");
  if (!cf) {
    perror("fopen");
    exit(-1);
  }

  int nread;
  char *child_str = NULL;
  size_t bufsz;
  do {
    nread = getdelim(&child_str, &bufsz, ' ', cf);
    if (nread == -1)
      break;

    int cpid = atoi(child_str);
    tree *ctree = mksubtree(cpid);

    // Capture any child processes with ptrace which are already alive at
    // startup. Any others made later will be caught automatically.
    if (ptrace(PTRACE_SEIZE, cpid, 0,
               PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK |
                   PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT) == -1) {
      perror("ptrace seize");
      exit(EXIT_FAILURE);
    }
    ll_push_back(&root->children, ctree);
    initialise_tree(ctree);
  } while (1);

  free(child_str);
  fclose(cf);
}

void tracer(pid_t rootpid) {
  tree *root = mksubtree(rootpid);
  initialise_tree(root);
  int status;

  if (ptrace(PTRACE_SEIZE, rootpid, 0,
             PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK |
                 PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT) == -1) {
    perror("ptrace seize");
    exit(EXIT_FAILURE);
  }

  screenfull(root);

  while (1) {
    pid_t wpid = waitpid(-1, &status, __WALL);
    if (wpid == -1) {
      if (errno == EINTR)
        continue;
      perror("waitpid");
      break;
    }

    if (WIFEXITED(status)) {
      treedel(root, wpid);
      push_history(EV_EXIT, wpid, -1, NULL, NULL);
      screenfull(root);
      continue;
    }

    if (WIFSTOPPED(status)) {
      int sig = WSTOPSIG(status);

      if ((status >> 16) == PTRACE_EVENT_FORK) {
        unsigned long new_pid;
        ptrace(PTRACE_GETEVENTMSG, wpid, 0, &new_pid);
        treeadd(root, wpid, new_pid);
        push_history(EV_FORK, wpid, new_pid, NULL, NULL);
        screenfull(root);
        // } else if ((status >> 16) == PTRACE_EVENT_VFORK) {
        //   unsigned long new_pid;
        //   ptrace(PTRACE_GETEVENTMSG, wpid, 0, &new_pid);
        //   printf("[VFORK] parent=%d child=%lu\n", wpid, new_pid);
        // } else if ((status >> 16) == PTRACE_EVENT_CLONE) {
        //   unsigned long new_pid;
        //   ptrace(PTRACE_GETEVENTMSG, wpid, 0, &new_pid);
        //   printf("[CLONE] parent=%d child=%lu\n", wpid, new_pid);
      } else if ((status >> 16) == PTRACE_EVENT_EXEC) {
        push_history(EV_EXEC, wpid, -1, NULL, get_cmdline(wpid));
        screenfull(root);
      }

      if (ptrace(PTRACE_CONT, wpid, 0, 0) == -1) {
        perror("ptrace cont");
        break;
      }
    }
  }
}
