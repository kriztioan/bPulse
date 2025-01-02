/**
 *  @file   ApplicationManager.h
 *  @brief  Application Manager Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef APPLICATIONMANAGER_H_
#define APPLICATIONMANAGER_H_

#include <functional>
#include <utility>
#include <vector>

#include <sys/select.h>
#include <sys/time.h>

#include <errno.h>
#include <signal.h>

class ApplicationManager {

public:
  ApplicationManager();

  ApplicationManager(int argc, char *argv[]);

  int RegisterCallback(std::function<int(void)> callback);

  int RegisterEventHandler(int fd, std::function<int(void)> handler);

  int UnRegisterEventHandler(int fd);

  int RegisterSignalHandler(int signal, std::function<int(int)> handler);

  int UnRegisterSignalHandler(int signal);

  int SetTimeout(int msec);

  int RunLoop();

  int TerminateLoop();

  static void SignalHandler(int sig);

  static int CaughtSignal;

private:
  int _init(int argc, char *argv[]);

  std::vector<std::function<int(void)>> _Callbacks;

  std::vector<std::pair<int, std::function<int(void)>>> _EventHandlers;

  std::vector<std::pair<int, std::function<int(int)>>> _SignalHandlers;

  int _RefreshInterval;

  bool _finished;

  struct timeval _timeout;
};

inline int ApplicationManager::TerminateLoop() {

  _finished = true;

  return 0;
}

inline int ApplicationManager::SetTimeout(int msec) {

  timerclear(&_timeout);

  _timeout.tv_sec = msec / 1000;

  _timeout.tv_usec = (msec % 1000) * 1000;

  return 0;
}
#endif // End of APPLICATIONMANAGER_H_
