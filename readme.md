# rptree

A real-time process tree viewer and fork/exec/exit event monitor.

## Overview

**rptree** attaches to a process using the ptrace interface, and displays a live-updating view of the process tree originating on the chosen parent. It also displays a log of _events_: when processes call `fork()`, `exec()`, or `exit()`.

<script src="https://asciinema.org/a/Rf1mGtrqevPuBgTfP6irYPSwb.js" id="asciicast-Rf1mGtrqevPuBgTfP6irYPSwb" async="true"></script>

The above recording shows a short demonstration of **rptree** tracing a bash shell. You can see fork events and processes in the tree whenever bash forks. Note the command with pipelines shows the pipes in the file descriptor fields of the table, and the redirected command to a file shows that as well.

Although **rptree** is run with sudo in the recording, you **do not need sudo** to use rptree, as long as you have control of starting the process to monitor.

## Why?

It can be useful for debugging, especially for developing things like shells. The specific use case I made this for is the Operating Systems course at Chalmers University. I'm a teaching assistant in that course, and I think/hope that it will help with understanding the task to implement a shell.

It can also be pretty useful for just seeing what processes bash makes. Try watching `man gcc`, for example, and see how many processes are spawned. Or, how many processes would you expect to be created when bash simply runs `ls`?

## Usage

### Installation

**rptree** doesn't use any fancy dependencies. Simply clone or download the repo on a Linux system, run `make`, and you'll be left with an executable called `rptree`.

I've also provided an `rptree_completion.sh`, which you can source from your bash shell to provide command-line autocompletion of PIDs when invoking rptree. It's a bit nice to use.

For system installation, do this:

```sh
make
cp rptree /usr/bin
cp rptree_completion.sh /etc/bash_completion.d
```

You'll probably need to run those with `sudo`. You can of course run it locally without installing system-wide.

### Using **rptree**

**rptree** uses `ptrace` to monitor your process. By default, you will need sudo (or similar) to attach to arbitrary processes. If you have access to sudo, simply run `sudo rptree <the pid of your process>`, and everything should work.

### Without sudo

In short, do this:

```sh
./traceme <your program>
# then, from another terminal,
./rptree <the pid which traceme reported>
```

Luckily, it's possible to attach without using sudo. The idea is that a child can call `prctl(PR_SET_TRACER, PR_SET_PTRACER_ANY)` on itself to allow any process to trace it. If you run your process-to-monitor with the included `traceme` program (e.g. `traceme bash` to run bash with tracing globally allowed), then you can `rptree` it without sudo!

Be careful though! This method allows _any_ other process on the system, so it's recommended not to do this with sensitive programs, and/or on systems which others are using.

The reason `traceme` uses `PR_SET_PTRACER_ANY` rather than specifically allowing `rptree` is just because it's way less fiddly.

## Related Work

 - **(h)top** provides similar features to rptree, in that it can display a process tree in real time. It doesn't, however, provide a simple way to view and follow the tree of one specific process, as far as I can tell. It also doesn't show the log of events, which can be really useful for debugging.
 - **pstree** is quite similar, superficially, but with major differences. Aside from the fact that pstree also does not show a log of ptrace events, it also is not a real-time view, it just prints the tree and exits. You could do something like `watch pstree`, but you will then miss quickly exitting processes.

## Future Work

There are quite a few things that would further improve this.

 - Save the tree state after each event, and use the up and down arrow keys to view historical tree states rather than the live one.
 - When viewing historical trees, as well as in general if desired, it would be good to be able to freeze the whole process tree's execution.
 - It's not easy at a glance to distinguish between all the different PIDs.
   - Option A: colour-code the PIDs using random colours from a palette, so that you can more easily spot recurring PID actions.
   - Option B: based on the PIDs on the screen, highlight just the last few digits of each to emphasize the different part of the number.
 - Writing the history to a log file.
 - Options to change filtering specific event types. Maybe you only want to see execs, for example.
 - It would be nice to show the first arg of each process' commandline next to EVERY instance of its PID being shown.
