#include "app_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

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

void AppLoader::getReservedMemRange(std::pair<void *, void *> &range)
{
  Area area;
  bool found = false;
  int mapsfd = open("/proc/self/maps", O_RDONLY);
  if (mapsfd < 0)
  {
    DLOG(ERROR, "Failed to open proc maps\n");
    return;
  }
  while (readMapsLine(mapsfd, &area))
  {
    if (strstr(area.name, "[stack]"))
    {
      found = true;
      break;
    }
  }
  close(mapsfd);

  // if (found && (g_range == nullptr))
  if (found)
  {
    reserved_area->start = (VA)area.addr - _3_GB;
    reserved_area->end = (VA)area.addr - _1_GB;
    range.first = reserved_area->start;
    range.second = reserved_area->end;
  }  
}

int AppLoader::readMapsLine(int mapsfd, Area *area)
{
  char c, rflag, sflag, wflag, xflag;
  int i;
  off_t offset;
  unsigned int long devmajor, devminor, inodenum;
  VA startaddr, endaddr;

  c = readHex(mapsfd, &startaddr);
  if (c != '-')
  {
    if ((c == 0) && (startaddr == 0))
      return (0);
    goto skipeol;
  }
  c = readHex(mapsfd, &endaddr);
  if (c != ' ')
    goto skipeol;
  if (endaddr < startaddr)
    goto skipeol;

  rflag = c = readChar(mapsfd);
  if ((c != 'r') && (c != '-'))
    goto skipeol;
  wflag = c = readChar(mapsfd);
  if ((c != 'w') && (c != '-'))
    goto skipeol;
  xflag = c = readChar(mapsfd);
  if ((c != 'x') && (c != '-'))
    goto skipeol;
  sflag = c = readChar(mapsfd);
  if ((c != 's') && (c != 'p'))
    goto skipeol;

  c = readChar(mapsfd);
  if (c != ' ')
    goto skipeol;

  c = readHex(mapsfd, (VA *)&offset);
  if (c != ' ')
    goto skipeol;
  area->offset = offset;

  c = readHex(mapsfd, (VA *)&devmajor);
  if (c != ':')
    goto skipeol;
  c = readHex(mapsfd, (VA *)&devminor);
  if (c != ' ')
    goto skipeol;
  c = readDec(mapsfd, (VA *)&inodenum);
  area->name[0] = '\0';
  while (c == ' ')
    c = readChar(mapsfd);
  if (c == '/' || c == '[')
  { /* absolute pathname, or [stack], [vdso], etc. */
    i = 0;
    do
    {
      area->name[i++] = c;
      if (i == sizeof area->name)
        goto skipeol;
      c = readChar(mapsfd);
    } while (c != '\n');
    area->name[i] = '\0';
  }

  if (c != '\n')
    goto skipeol;

  area->addr = startaddr;
  area->endAddr = endaddr;
  area->size = endaddr - startaddr;
  area->prot = 0;
  if (rflag == 'r')
    area->prot |= PROT_READ;
  if (wflag == 'w')
    area->prot |= PROT_WRITE;
  if (xflag == 'x')
    area->prot |= PROT_EXEC;
  area->flags = MAP_FIXED;
  if (sflag == 's')
    area->flags |= MAP_SHARED;
  if (sflag == 'p')
    area->flags |= MAP_PRIVATE;
  if (area->name[0] == '\0')
    area->flags |= MAP_ANONYMOUS;

  area->devmajor = devmajor;
  area->devminor = devminor;
  area->inodenum = inodenum;
  return (1);

skipeol:
  fprintf(stderr, "ERROR: readMapsLine*: bad maps line <%c", c);
  while ((c != '\n') && (c != '\0'))
  {
    c = readChar(mapsfd);
    printf("%c", c);
  }
  printf(">\n");
  abort();
  return 0; /* NOTREACHED : stop compiler warning */
}