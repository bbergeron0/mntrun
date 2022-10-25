/* See LICENSE file for copyright and license details. */
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "mount.c"

#ifndef MNTRUN_VERSION
#define MNTRUN_VERSION "(unknown)"
#endif

#define USAGE "usage: mntrun [-dhv] [-b source dest] [[-o|-m] upperdir lowerdir workdir mountpoint] command\n"

static inline int checkflag(char *arg, char f) {
    return arg && (arg[0] == '-' && arg[1] == f && arg[2] == '\0');
}

int namespace_main(void *data) {
    uid_t uid = getuid();
    gid_t gid = getgid();

    int verbose = 0;
    char **argv = data;

    if(checkflag(argv[0], 'h')) {
        fputs(USAGE, stderr);
        return EXIT_SUCCESS;
    }

    if(checkflag(argv[0], 'v')) {
        puts("mntrun " MNTRUN_VERSION);
        return EXIT_SUCCESS;
    }

    if(checkflag(argv[0], 'd')) {
        verbose = 1;
        argv ++;
    }

    /* Jump back here over-and-over until all mount directives are parsed. */
    do_mounts:
    if(checkflag(argv[0], 'b')) {
        if(!argv[1] || !argv[2]) goto exit_usage;

        if(mount_bind(argv[1], argv[2]) == -1) {
            if(errno) fprintf(stderr, "Failed to bind '%s' to '%s': %s\n", argv[1], argv[2], strerror(errno));
            return EXIT_FAILURE;
        }
        argv += 3;
        goto do_mounts;
    }

    int merge = 0;
    if(checkflag(argv[0], 'o') || (merge = checkflag(argv[0], 'm'))) {
        char *type_str = merge ? "merge" : "overlay";

        if(!argv[1] || !argv[2] || !argv[3] || !argv[4]) goto exit_usage;
        if(verbose) printf("[%s]\n\tlower=%s\n\tupper=%s\n\twork=%s\n\tmount=%s\n", type_str, argv[1], argv[2], argv[3], argv[4]);

        char *lowers = argv[1];
        if(merge) {
            lowers = malloc(strlen(argv[4]) + 1 + strlen(argv[1]));
            sprintf(lowers, "%s:%s", argv[4], argv[1]);
        }

        if(mount_overlay(uid, gid, lowers, argv[2], argv[3], argv[4]) == -1) {
            if(errno) fprintf(stderr, "Failed to mount %s on '%s': %s\n", type_str, argv[4], strerror(errno));
            return EXIT_FAILURE;
        }
        argv += 5;
        if(merge) {
            free(lowers);
            merge = 0;
        }
        goto do_mounts;
    }
    /* Check if a command was specified */
    if(!argv[0]) goto exit_usage;

    setuid(uid);

    /* These next lines take the remaining values in `argv` and join them with spaces in `command`. */
    int charc;
    for(int i = 0; argv[i]; i ++) charc += strlen(argv[i]);
    char *command = malloc(charc + 1);
    command[0] = '\0';
    for(int i = 0; argv[i]; i ++) {
        strcat(command, " ");
        strcat(command, argv[i]);
    }
    
    int status = system(command);
    if(status == -1) {
        perror("system");
        return EXIT_FAILURE;
    }
    return status;

exit_usage:
    fputs(USAGE, stderr);
    return EXIT_FAILURE;
}

int main(int argc, char **argv) {
    #define STACK_SIZE 1024 * 1024
    char *stack = malloc(STACK_SIZE);
    pid_t pid = clone(namespace_main, stack + STACK_SIZE, CLONE_NEWNS | SIGCHLD, argv + 1);
    if(pid == -1) {
        perror("Failed to create a new mount namespace");
        return EXIT_FAILURE;
    }
    
    int wstatus;
    if (waitpid(pid, &wstatus, 0) == -1) {
        /* The only error which may occurs is EINTR, but I'm not 100% sure. */
        perror("waitpid");
        return EXIT_FAILURE;
    }
    return WEXITSTATUS(wstatus);
}
