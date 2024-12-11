/**
 *  @file   ProcManager.cpp
 *  @brief  MacOS Process Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "ProcManager.h"

ProcManager::ProcManager() { _init(0, nullptr); }

ProcManager::ProcManager(int argc, char *argv[]) { _init(argc, argv); }

ProcManager::~ProcManager() {
  _terminate_probe_thread = true;
  _probe_condition.notify_one();
  _probe_thread.join();
}

int ProcManager::_init(int argc, char *argv[]) {

  _mask = static_cast<int>(ProcManager::Masks::Ignore);

  memset(&proc_cpu1, 0, sizeof(struct s_pcpu));

  memset(&proc_cpu2, 0, sizeof(struct s_pcpu));

  memset(&proc_eth1, 0, sizeof(struct s_peth));

  memset(&proc_eth2, 0, sizeof(struct s_peth));

  memset(&proc_io1, 0, sizeof(struct s_peth));

  memset(&proc_io2, 0, sizeof(struct s_pio));

  _probe_thread = std::thread(&ProcManager::_probe_thread_func, this);

  return 0;
}

void ProcManager::Probe() {
  _probe_execute = true;
  _probe_condition.notify_one();
}

void ProcManager::_probe_thread_func() {
 while (true) {
    std::unique_lock<std::mutex> lock(_probe_mutex);
    _probe_condition.wait(lock, [&] { return _probe_execute || _terminate_probe_thread; });
    if(_terminate_probe_thread) {
       break;
    }

     _probe();

    _probe_execute = false;
  }
}

int ProcManager::_probe() {

  if (_mask & Masks::CPU) {

    processor_cpu_load_info_t cpu_info;

    mach_msg_type_number_t n_cpu_info;

    natural_t ncpu = 0U;

    kern_return_t err =
        host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &ncpu,
                            (processor_info_array_t *)&cpu_info, &n_cpu_info);

    if (KERN_SUCCESS == err) {

      memset(&proc_cpu2, 0, sizeof(struct s_pcpu));

      while (ncpu--) {

        proc_cpu2.user +=
            static_cast<float>(cpu_info[ncpu].cpu_ticks[CPU_STATE_USER]);
        proc_cpu2.nice +=
            static_cast<float>(cpu_info[ncpu].cpu_ticks[CPU_STATE_NICE]);
        proc_cpu2.sys +=
            static_cast<float>(cpu_info[ncpu].cpu_ticks[CPU_STATE_SYSTEM]);
        proc_cpu2.idle +=
            static_cast<float>(cpu_info[ncpu].cpu_ticks[CPU_STATE_IDLE]);
      }
    }

    proc_cpu2.total =
        proc_cpu2.user + proc_cpu2.nice + proc_cpu2.sys + proc_cpu2.idle;

    cpu.user = static_cast<float>(proc_cpu2.user - proc_cpu1.user) /
               (proc_cpu2.total - proc_cpu1.total);

    cpu.nice = static_cast<float>(proc_cpu2.nice - proc_cpu1.nice) /
               (proc_cpu2.total - proc_cpu1.total);

    cpu.sys = static_cast<float>(proc_cpu2.sys - proc_cpu1.sys) /
              (proc_cpu2.total - proc_cpu1.total);

    cpu.idle = static_cast<float>(proc_cpu2.idle - proc_cpu1.idle) /
               (proc_cpu2.total - proc_cpu1.total);

    proc_cpu1 = proc_cpu2;
  }

  if (_mask & Masks::Eth) {

    unsigned int ifindex = if_nametoindex(_eth.c_str());

    int mib[6] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, (int)ifindex};

    size_t n;

    if (0 == sysctl(mib, sizeof(mib) / sizeof(int), NULL, &n, NULL, 0)) {

      char *records = (char *)malloc(n), *record;

      if (0 == sysctl(mib, sizeof(mib) / sizeof(int), records, &n, NULL, 0)) {

        struct if_msghdr2 if_msghdr2_s;

        for (record = records; record < records + n;
             record += if_msghdr2_s.ifm_msglen) {

          memcpy(&if_msghdr2_s, record, sizeof(struct if_msghdr2));

          if (ifindex != if_msghdr2_s.ifm_index ||
              RTM_IFINFO2 != if_msghdr2_s.ifm_type ||
              (if_msghdr2_s.ifm_flags & IFF_LOOPBACK)) {

            continue;
          }

          proc_eth2.received = if_msghdr2_s.ifm_data.ifi_ibytes;
          proc_eth2.sent = if_msghdr2_s.ifm_data.ifi_obytes;
        }
      }

      free(records);
    }

    eth.received = proc_eth2.received - proc_eth1.received;

    eth.sent = proc_eth2.sent - proc_eth1.sent;

    proc_eth1 = proc_eth2;
  }

  if (_mask & Masks::IO) {

    mach_port_t master_port = kIOMainPortDefault;

    CFMutableDictionaryRef match = IOServiceMatching("IOMedia");

    CFDictionaryAddValue(match, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);

    io_iterator_t drive_list;

    kern_return_t status =
        IOServiceGetMatchingServices(master_port, match, &drive_list);

    if (status == KERN_SUCCESS) {

      CFMutableDictionaryRef match = IOServiceMatching("IOMedia");

      CFDictionaryAddValue(match, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);

      io_iterator_t drive_list;

      kern_return_t status =
          IOServiceGetMatchingServices(master_port, match, &drive_list);

      if (status == KERN_SUCCESS) {

        io_registry_entry_t parent, drive;

        CFNumberRef nValue;

        long io_read, io_write, io_read_total = 0L, io_write_total = 0L;

        memset(&proc_io2, 0, sizeof(struct s_pio));

        while ((drive = IOIteratorNext(drive_list))) {

          status =
              IORegistryEntryGetParentEntry(drive, kIOServicePlane, &parent);

          if (status == KERN_SUCCESS) {

            if (IOObjectConformsTo(parent, "IOBlockStorageDriver")) {

              CFMutableDictionaryRef properties;

              CFDictionaryRef stats;

              status = IORegistryEntryCreateCFProperties(
                  parent, (CFMutableDictionaryRef *)&properties,
                  kCFAllocatorDefault, kNilOptions);

              if (status != KERN_SUCCESS) {

                IOObjectRelease(parent);

                IOObjectRelease(drive);

                CFRelease(properties);

                continue;
              }

              stats = (CFDictionaryRef)CFDictionaryGetValue(
                  properties, CFSTR(kIOBlockStorageDriverStatisticsKey));

              if (!stats) {

                IOObjectRelease(parent);

                IOObjectRelease(drive);

                CFRelease(properties);

                continue;
              }

              nValue = (CFNumberRef)CFDictionaryGetValue(
                  stats, CFSTR(kIOBlockStorageDriverStatisticsBytesReadKey));

              CFNumberGetValue(nValue, kCFNumberSInt64Type, &io_read);

              proc_io2.read += io_read;

              nValue = (CFNumberRef)CFDictionaryGetValue(
                  stats, CFSTR(kIOBlockStorageDriverStatisticsBytesWrittenKey));

              CFNumberGetValue(nValue, kCFNumberSInt64Type, &io_write);

              proc_io2.write += io_write;

              IOObjectRelease(parent);

              IOObjectRelease(drive);

              CFRelease(properties);
            }
          }
        }
      }
    }

    IOObjectRelease(drive_list);

    io.read = proc_io2.read - proc_io1.read;

    io.write = proc_io2.write - proc_io1.write;

    proc_io1 = proc_io2;
  }

  if (_mask & Masks::EMail) {

    // Do Email
  }

  if (_mask & Masks::Host) {

    char *user = getenv("USER"), *host = getenv("HOST");

    if (user != nullptr) {

      _host = std::string(user);
    }

    if (host != nullptr) {

      _host += '@';

      _host += std::string();
    }
  }

  if (_mask & Masks::Alarm) {

    // Do Alarm
  }

  if (_mask & Masks::Mem) {

    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;

    vm_statistics_data_t vm_stat;

    kern_return_t err = host_statistics(mach_host_self(), HOST_VM_INFO,
                                        (host_info_t)&vm_stat, &count);
    if (KERN_SUCCESS == err) {

      struct sysinfo {
        unsigned long totalram;  /* Total usable main memory size */
        unsigned long freeram;   /* Available memory size */
        unsigned long sharedram; /* Amount of shared memory */
        unsigned long bufferram; /* Memory used by buffers */
      };

      memory.freeram = vm_stat.free_count;
      memory.totalram = vm_stat.active_count + vm_stat.inactive_count +
                        vm_stat.wire_count + vm_stat.free_count;
      memory.sharedram = vm_stat.active_count;
      memory.bufferram = vm_stat.inactive_count;
    }
  }

  if (_mask & Masks::Disk) {

    if (statfs(_disk.c_str(), &disk) != 0) {

      return 1;
    }
  }

  if (_mask & Masks::Users) {

    setutxent();

    struct utmpx *s_utmpx;

    while ((s_utmpx = getutxent()) != NULL) {

      if (s_utmpx->ut_type == USER_PROCESS) {

        int s;

        for (s = users.size(); s--;) {

          if (std::find(users.begin(), users.end(),
                        std::string(s_utmpx->ut_user)) != users.end()) {

            break;
          }
        }

        if (s < 0) {

          users.emplace_back(s_utmpx->ut_user);
        }
      }
    }

    endutxent();
  }

  if (_mask & Masks::Battery) {

    CFTypeRef blob = IOPSCopyPowerSourcesInfo();

    CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

    if (CFArrayGetCount(sources) == 0) {

      return -1;
    }

    CFDictionaryRef source =
        IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, 0));

    if (NULL == source) {

      return -1;
    }

    CFNumberRef nValue;

    long currentCapacity, maxCapacity;

    nValue = (CFNumberRef)CFDictionaryGetValue(source,
                                               CFSTR(kIOPSCurrentCapacityKey));

    CFNumberGetValue(nValue, kCFNumberSInt64Type, &currentCapacity);

    nValue =
        (CFNumberRef)CFDictionaryGetValue(source, CFSTR(kIOPSMaxCapacityKey));

    CFNumberGetValue(nValue, kCFNumberSInt64Type, &maxCapacity);

    battery = {.powerstate = PowerStates::Unknown,
               .level = static_cast<float>(currentCapacity) /
                        static_cast<float>(maxCapacity)};

    CFStringRef sValue = (CFStringRef)CFDictionaryGetValue(
        source, CFSTR(kIOPSPowerSourceStateKey));

    if (kCFCompareEqualTo ==
        CFStringCompare(CFSTR(kIOPSACPowerValue), sValue, 0)) {

      nValue = (CFNumberRef)CFDictionaryGetValue(
          source, CFSTR(kIOPSTimeToFullChargeKey));

      CFBooleanRef bValue =
          (CFBooleanRef)CFDictionaryGetValue(source, CFSTR(kIOPSIsChargingKey));

      if (CFBooleanGetValue(bValue)) {

        battery.powerstate = PowerStates::BatteryCharging;
      } else {

        battery.powerstate = PowerStates::ACPower;
      }
    } else if (kCFCompareEqualTo ==
               CFStringCompare(CFSTR(kIOPSBatteryPowerValue), sValue, 0)) {

      nValue =
          (CFNumberRef)CFDictionaryGetValue(source, CFSTR(kIOPSTimeToEmptyKey));

      battery.powerstate = PowerStates::BatteryDischarging;
    }

    CFRelease(blob);

    CFRelease(sources);
  }

  return 0;
}
