#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s CMD [ARGS...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("# (traceme) Attach with `rptree %d`\n", getpid());
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
    execvp(argv[1], argv + 1);
}
