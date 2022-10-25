mntrun - mounts binds and overlays inside an ephemeral mount namespace, and runs a command in it.

Usage: `mntrun [-dhv] [-b source dest] [-o|-m lower upper workdir mountpoint] command`

This tiny program exploits SUID to allow regular users to create overlays and binds mounts inside an ephemeral namespace, and to run `command` inside this new namespace.

For further usage documentation, consult the dedicated man page.

mntrun is fewer than 200 lines of fairly simple C99. `mount.c` contains all (2) mount wrappers, while `main.c` contains `main` and `namespace_main`, which does what you expect.

glibc Linux only (`clone(2)` seems to be non-standard, sorry)