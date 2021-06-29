#include "app_loader.h"
#include <limits.h>
#include <stdio.h>

// Returns the address of argc on the stack
unsigned long AppLoader::getStackPtr()
{
    // From man 5 proc: See entry for /proc/[pid]/stat
    int pid;
    char cmd[PATH_MAX];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long long starttime;
    unsigned long vsize;
    long rss;
    unsigned long rsslim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal_map;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned rt_priority;
    unsigned policy;

    FILE *f = fopen("/proc/self/stat", "r");
    if (f)
    {
        fscanf(f, "%d "
                  "%s %c "
                  "%d %d %d %d %d "
                  "%u "
                  "%lu %lu %lu %lu %lu %lu "
                  "%ld %ld %ld %ld %ld %ld "
                  "%llu "
                  "%lu "
                  "%ld "
                  "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
                  "%d %d %u %u",
               &pid,
               cmd, &state,
               &ppid, &pgrp, &session, &tty_nr, &tpgid,
               &flags,
               &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime,
               &cutime, &cstime, &priority, &nice, &num_threads, &itrealvalue,
               &starttime,
               &vsize,
               &rss,
               &rsslim, &startcode, &endcode, &startstack, &kstkesp, &kstkeip,
               &signal_map, &blocked, &sigignore, &sigcatch, &wchan, &nswap,
               &cnswap,
               &exit_signal, &processor,
               &rt_priority, &policy);
        fclose(f);
    }
    return startstack;
}