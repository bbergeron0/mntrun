/* See LICENSE file for copyright and license details. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

int mount_overlay(uid_t uid, gid_t gid, const char *restrict lower, const char *restrict upper, const char *restrict workdir, const char *restrict mountpoint) {
    if(access(mountpoint, W_OK) || access(upper, W_OK) || access(workdir, W_OK)) return -1;

    char *options = malloc(
        sizeof("lowerdir=") + strlen(lower) +
        sizeof(",upperdir=") + strlen(upper) +
        sizeof(",workdir=") + strlen(workdir)
    );
    sprintf(options, "lowerdir=%s,upperdir=%s,workdir=%s", lower, upper, workdir);
    int mount_status = mount("overlay", mountpoint, "overlay", 0, options);
    free(options);
    if(mount_status) return -1;

    /* When mounting an overlay, a folder named 'work' belonging to root:root is created inside workdirt. These next lines give the ownership back to the current user. */
    char *other_work_dir = malloc(strlen(workdir) + sizeof("/work"));
    sprintf(other_work_dir, "%s/work", workdir);
    int status = chown(other_work_dir, uid, gid);
    free(other_work_dir);
    return status;
}

int mount_bind(const char *restrict source, const char *restrict dest) {
    if(access(dest, W_OK)) return -1;
    return mount(source, dest, "none", MS_BIND, NULL);
}
