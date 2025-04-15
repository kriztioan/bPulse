/**
 *  @file   ProcManager.cpp
 *  @brief  Linux Process Manager Class Implementation
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

  proc_cpu1 = {0};

  proc_cpu2 = {0};

  proc_eth1 = {0};

  proc_eth2 = {0};

  proc_io1 = {0};

  proc_io2 = {0};

  _probe_thread = std::thread(&ProcManager::_probe_thread_func, this);

  return 0;
}

void ProcManager::Probe() {

  if (_probe_execute) {

    std::unique_lock<std::mutex> lock(_probe_mutex);

    _probe_condition.wait(lock, [&] { return !_probe_execute; });
  }

  _probe_execute = true;

  _probe_condition.notify_one();
}

void ProcManager::_probe_thread_func() {

  while (true) {

    std::unique_lock<std::mutex> lock(_probe_mutex);

    _probe_condition.wait(
        lock, [&] { return _probe_execute || _terminate_probe_thread; });

    if (_terminate_probe_thread) {
      break;
    }

    _probe();

    _probe_execute = false;

    _probe_condition.notify_one();
  }
}

int ProcManager::_probe() {

  if (_mask & Masks::CPU) {

    std::ifstream fstream("/proc/stat", std::ios::in);

    if (fstream.fail()) {

      return 1;
    }

    std::string line;

    while (std::getline(fstream, line)) {

      if (line.find(_cpu) != std::string::npos) {

        break;
      }
    }

    sscanf(line.c_str(), "%*s %lu %lu %lu %lu", &proc_cpu2.user,
           &proc_cpu2.nice, &proc_cpu2.sys, &proc_cpu2.idle);

    fstream.close();

    proc_cpu2.total =
        proc_cpu2.user + proc_cpu2.nice + proc_cpu2.sys + proc_cpu2.idle;

    float diff = proc_cpu2.total - proc_cpu1.total;

    if (diff > 0.0f) {

      cpu.user = (proc_cpu2.user - proc_cpu1.user) / diff;

      cpu.nice = (proc_cpu2.nice - proc_cpu1.nice) / diff;

      cpu.sys = (proc_cpu2.sys - proc_cpu1.sys) / diff;

      cpu.idle = (proc_cpu2.idle - proc_cpu1.idle) / diff;

      proc_cpu1 = proc_cpu2;
    }
  }

  if (_mask & Masks::Eth) {

    std::ifstream fstream("/proc/net/dev", std::ios::in);

    if (fstream.fail()) {

      return 1;
    }

    std::string line;

    while (std::getline(fstream, line)) {

      if (line.find(_eth) != std::string::npos) {

        break;
      }
    }

    sscanf(line.c_str(), " %*s %lu %*d %*d %*d %*d %*d %*d %*d %lu",
           &proc_eth2.received, &proc_eth2.sent);

    fstream.close();

    eth.received = proc_eth2.received - proc_eth1.received;

    eth.sent = proc_eth2.sent - proc_eth1.sent;

    proc_eth1 = proc_eth2;
  }

  if (_mask & Masks::IO) {

    std::ifstream fstream("/proc/diskstats", std::ios::in);

    if (fstream.fail()) {

      return 1;
    }

    std::string line;

    while (std::getline(fstream, line)) {

      if (line.find(_io) != std::string::npos) {

        break;
      }
    }

    sscanf(line.c_str(), "%*d %*d %*s %*d %*d %lu %*d %*d %*d %lu",
           &proc_io2.read, &proc_io2.write);

    fstream.close();

    io.read = 512 * (proc_io2.read - proc_io1.read);

    io.write = 512 * (proc_io2.write - proc_io1.write);

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

    if (sysinfo(&memory) == -1) {

      return 1;
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

          users.push_back(std::string(s_utmpx->ut_user));
        }
      }
    }

    endutxent();
  }

  if (_mask & Masks::Battery) {

    // Do battery
    battery.level = -1;

    battery.powerstate = PowerStates::Unknown;
  }

  return 0;
}
