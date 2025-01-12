/**
 *  @file   ProcManager.h
 *  @brief  Linux Process Manager Class Definition
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
#include <sys/statfs.h>
#include <sys/sysinfo.h>

#include <utmpx.h>

#include <cstring>

#include <string>

#include <fstream>

#include <vector>

#include <algorithm>

#include <condition_variable>

#include <thread>
class ProcManager {

public:
  enum class Masks {
    Ignore = 0L,
    CPU = 1L,
    Mem = 1L << 2,
    IO = 1L << 3,
    Eth = 1L << 4,
    EMail = 1L << 5,
    Disk = 1L << 6,
    Alarm = 1L << 7,
    Host = 1L << 8,
    Users = 1L << 9,
    Battery = 1L << 10,
    All = 1L << 12 
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

  enum class PowerStates {
    Unknown,
    ACPower,
    BatteryCharging,
    BatteryDischarging,
    All = 512L
  };

  struct s_battery {
    PowerStates powerstate;
    float level;
  } battery;

  std::vector<std::string> users;

  ProcManager();

  ProcManager(int argc, char *argv[]);

  ~ProcManager();

  int SetProcMask(int mask);

  int SetDisk(const char *path);

  int SetCPU(const char *cpu);

  int SetEth(const char *eth);

  int SetIO(const char *io);

  void  Probe();

private:
  int _init(int argc, char *argv[]);

  int _probe();

  void _probe_thread_func();

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

  std::thread _probe_thread;

  std::condition_variable _probe_condition;

  std::mutex _probe_mutex;

  bool _terminate_probe_thread = false;

  bool _probe_execute = false;
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
