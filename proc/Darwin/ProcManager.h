/**
 *  @file   ProcManager.h
 *  @brief  MacOS Process Manager Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef PROCMANAGER_H_
#define PROCMANAGER_H_

#include <cstdlib>

#include <sys/mount.h>
#include <sys/param.h>

#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_mib.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <net/route.h>

#include <utmpx.h>

#include <cstring>

#include <string>

#include <fstream>

#include <vector>

#include <algorithm>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>

class ProcManager {

public:
  struct sysinfo {
    unsigned long totalram;  /* Total usable main memory size */
    unsigned long freeram;   /* Available memory size */
    unsigned long sharedram; /* Amount of shared memory */
    unsigned long bufferram; /* Memory used by buffers */
  };

  enum class Masks {
    Ignore = 0L,
    CPU,
    Mem,
    IO,
    Eth,
    EMail,
    Disk,
    Alarm,
    Host,
    Users,
    All = 512L
  };

  struct s_cpu {
    float user, nice, sys, idle;
  } cpu;

  struct sysinfo memory;

  struct statfs disk;

  struct s_eth {
    unsigned long received, sent;
  } eth;

  struct s_io {
    unsigned long read, write;
  } io;

  std::string host;

  std::vector<std::string> users;

  ProcManager();

  ProcManager(int argc, char *argv[]);

  int SetProcMask(int mask);

  int SetDisk(const char *path);

  int SetCPU(const char *cpu);

  int SetEth(const char *eth);

  int SetIO(const char *io);

  int Probe();

private:
  int _init(int argc, char *argv[]);

  int _mask;

  struct s_pcpu {

    unsigned long user, nice, sys, idle, total;
  } proc_cpu1, proc_cpu2;

  struct s_peth {

    unsigned long received, sent;
  } proc_eth1, proc_eth2;

  struct s_pio {
    unsigned long read, write;
  } proc_io1, proc_io2;

  std::string _disk;

  std::string _cpu;

  std::string _eth;

  std::string _io;

  std::string _host;
};

inline int operator&(int a, ProcManager::Masks b) {

  return a & static_cast<int>(b);
}

inline int operator|(int a, ProcManager::Masks b) {

  return a | static_cast<int>(b);
}

inline int operator|(ProcManager::Masks a, ProcManager::Masks b) {

  return static_cast<int>(a) | static_cast<int>(b);
}

inline int operator^(ProcManager::Masks a, ProcManager::Masks b) {

  return static_cast<int>(a) ^ static_cast<int>(b);
}

inline int ProcManager::SetDisk(const char *path) {

  _disk = std::string(path);

  return 0;
}

inline int ProcManager::SetCPU(const char *cpu) {

  _cpu = std::string(cpu);

  return 0;
}

inline int ProcManager::SetEth(const char *eth) {

  _eth = std::string(eth);

  return 0;
}

inline int ProcManager::SetIO(const char *io) {

  _io = std::string(io);

  return 0;
}

inline int ProcManager::SetProcMask(int mask) {

  _mask = mask;

  return 0;
}
#endif // End of PROCMANAGER_H_
