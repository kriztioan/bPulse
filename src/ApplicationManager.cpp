/**
 *  @file   ApplicationManager.cpp
 *  @brief  Application Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "ApplicationManager.h"

ApplicationManager::ApplicationManager() { _init(0, nullptr); }

ApplicationManager::ApplicationManager(int argc, char *argv[]) {

  _init(argc, argv);
}

int ApplicationManager::_init(int argc, char *argv[]) { return 0; }

int ApplicationManager::RegisterCallback(std::function<int(void)> callback) {

  _Callbacks.push_back(callback);

  return 0;
}

int ApplicationManager::RegisterEventHandler(int fd,
                                             std::function<int(void)> handler) {

  _EventHandlers.push_back(make_pair(fd, handler));

  return 0;
}

int ApplicationManager::UnRegisterEventHandler(int fd) {

  for (std::vector<std::pair<int, std::function<int(void)>>>::iterator
           eventhandler = _EventHandlers.begin();
       eventhandler != _EventHandlers.end(); ++eventhandler) {

    if (eventhandler->first == fd) {

      _EventHandlers.erase(eventhandler);

      break;
    }
  }

  return 0;
}

int ApplicationManager::RegisterSignalHandler(int signal,
                                              std::function<int(int)> handler) {

  _SignalHandlers.push_back(make_pair(signal, handler));

  return 0;
}

int ApplicationManager::UnRegisterSignalHandler(int signal) {

  for (std::vector<std::pair<int, std::function<int(int)>>>::iterator
           signalhandler = _SignalHandlers.begin();
       signalhandler != _SignalHandlers.end(); ++signalhandler) {

    if (signalhandler->first == signal) {

      _SignalHandlers.erase(signalhandler);

      break;
    }
  }

  return 0;
}

int ApplicationManager::CaughtSignal;

void ApplicationManager::SignalHandler(int sig) {

  ApplicationManager::CaughtSignal = sig;
}

int ApplicationManager::RunLoop() {

  struct timeval timeouttest, t0, t1, dt;

  timeouttest = _timeout;

  fd_set readfds, testfds;

  int maxfd = -1;

  FD_ZERO(&readfds);

  for (std::vector<std::pair<int, std::function<int(void)>>>::iterator
           eventhandler = _EventHandlers.begin();
       eventhandler != _EventHandlers.end(); ++eventhandler) {

    FD_SET(eventhandler->first, &readfds);

    if (eventhandler->first > maxfd) {

      maxfd = eventhandler->first;
    }
  }

  testfds = readfds;

  sigset_t sigmask, emptymask;

  sigemptyset(&sigmask);

  sigemptyset(&emptymask);

  struct sigaction sa;

  sigemptyset(&sa.sa_mask);

  sa.sa_flags = 0;

  for (std::vector<std::pair<int, std::function<int(int)>>>::iterator
           signalhandler = _SignalHandlers.begin();
       signalhandler != _SignalHandlers.end(); ++signalhandler) {

    sigaddset(&sigmask, signalhandler->first);

    sa.sa_handler = ApplicationManager::SignalHandler;

    sigaction(signalhandler->first, &sa, nullptr);
  }

  _finished = false;

  int ret = 0;

  while (!_finished) {

    if (sigprocmask(SIG_SETMASK, &emptymask, nullptr) == -1) {

      break;
    }

    if (gettimeofday(&t0, NULL) == -1) {

      break;
    }

    if ((ret = select(maxfd + 1, &testfds, nullptr, nullptr, &timeouttest)) <
        0) {

      break;
    }

    if (sigprocmask(SIG_SETMASK, &sigmask, nullptr) == -1) {

      break;
    }

    if (ret > 0) {

      for (std::vector<std::pair<int, std::function<int(void)>>>::iterator
               eventhandler = _EventHandlers.begin();
           eventhandler != _EventHandlers.end(); ++eventhandler) {

        if (FD_ISSET(eventhandler->first, &testfds)) {

          if ((ret = eventhandler->second()) != 0) {

            break;
          }
        }
      }

      if (gettimeofday(&t1, NULL) == -1) {

        break;
      }

      timersub(&t1, &t0, &dt);

      if (timercmp(&dt, &_timeout, <)) {

        timersub(&_timeout, &dt, &timeouttest);
      } else {

        ret = 0;
      }
    }

    if (ret == 0) {

      for (std::vector<std::function<int(void)>>::iterator callback =
               _Callbacks.begin();
           callback != _Callbacks.end(); ++callback) {

        if ((ret = (*callback)()) != 0) {

          break;
        }
      }

      if (gettimeofday(&t1, NULL) == -1) {

        break;
      }

      timersub(&t1, &t0, &dt);

      timeouttest = _timeout;
    }

    testfds = readfds;
  }

  if (errno == EINTR) {

    for (std::vector<std::pair<int, std::function<int(int)>>>::iterator
             signalhandler = _SignalHandlers.begin();
         signalhandler != _SignalHandlers.end(); ++signalhandler) {

      if (signalhandler->first == ApplicationManager::CaughtSignal) {

        ret = signalhandler->second(ApplicationManager::CaughtSignal);
      }
    }
  }

  return ret;
}
