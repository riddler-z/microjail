# Microjail

Microjail is a command-line tool, written in C, that runs any program inside a disposable, locked-down decoy environment. Prevents anything that runs in the environment from touching the real filesystem, network, and limits resource usage (i.e. CPU space), and kills anything malicious on-site!

why 'microjail'? micro, referring to a short-lived environment, and 'jail' to refer to how the environment isolates the task - a tiny jail for a task. How cute. 

## Milestone 1

**Using `fork()` to create the sandboxed process in the first place; the empty decoy that all later milestones get built around**

## Milestone 2

**Using `chroot()` to trap the sandboxed process inside a fake, minimal filesystem so it can't touch or see your real files**

## Milestone 3

**Using Linux namespaces so the sandboxed process gets its own private, empty view of processes, hostname, and network; therefore unable to see/reach anything on the real machine.**

## Milestone 4

**Using `cgroups` to cap how much CPU and the memory the sandboxed process is allowed to use; to prevent the host from being overwhelmed even if it tries.**

## Milestone 5

**Using `seccomp` to enforce an allow-list of `syscalls`, so the sandboxd process gets killed instantly if anything outside the defined scope is attempted.**

## Milestone 6

**Cleaning up everything including namespaces, cgroups, the jailed filesystem; the moment the sandboxed process exit, the decoy leaves no trace behind.**

## Milestone 7 (if time allows)

**Turning the above application into a reusable tool driven by a config file instead of hardcoded settings, with tests to prove it works.**