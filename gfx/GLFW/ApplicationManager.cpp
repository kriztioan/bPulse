/**
 *  @file   ApplicationManager.cpp
 *  @brief  Application Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-12-29
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "ApplicationManager.h"

#ifdef __APPLE__
int sigisemptyset(sigset_t *sigset) { return *(sigset) == 0; };
#endif

ApplicationManager::ApplicationManager() { _init(0, nullptr); }

ApplicationManager::ApplicationManager(int argc, char *argv[]) {

  _init(argc, argv);
}

int ApplicationManager::_init(int argc, char *argv[]) { return 0; }

int ApplicationManager::RegisterCallback(std::function<int(void)> callback) {

  _Callbacks.push_back(callback);

  return 0;
}

int ApplicationManager::RegisterEventHandler(GLFWwindow *window,
                                             std::function<int(void)> handler) {

  _EventHandlers.push_back(make_pair(window, handler));

  return 0;
}

int ApplicationManager::UnRegisterEventHandler(GLFWwindow *window) {

  for (std::vector<std::pair<GLFWwindow *, std::function<int(void)>>>::iterator
           eventhandler = _EventHandlers.begin();
       eventhandler != _EventHandlers.end(); ++eventhandler) {

    if (eventhandler->first == window) {

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

int ApplicationManager::RunLoop() {

  sigset_t sigmask;

  sigemptyset(&sigmask);

  struct sigaction sa;

  sigemptyset(&sa.sa_mask);

  sa.sa_flags = 0;

  for (std::vector<std::pair<int, std::function<int(int)>>>::iterator
           signalhandler = _SignalHandlers.begin();
       signalhandler != _SignalHandlers.end(); ++signalhandler) {

    sigaddset(&sigmask, signalhandler->first);
  }

  sigprocmask(SIG_SETMASK, &sigmask, nullptr);

  sigemptyset(&sigmask);

  int ret = 0;

  double frame_time = glfwGetTime(), delay_time = 0.0;

  while (!_finished) {

    if (sigpending(&sigmask) < 0) {

      break;
    }

    if (!sigisemptyset(&sigmask)) {

      break;
    }

    if ((glfwGetTime() - frame_time) >= timeout) {

      frame_time = glfwGetTime();

      for (std::vector<std::function<int(void)>>::iterator callback =
               _Callbacks.begin();
           callback != _Callbacks.end(); ++callback) {

        if ((ret = (*callback)()) != 0) {

          break;
        }
      }
    }

    delay_time = timeout - (glfwGetTime() - frame_time);

    if (delay_time <= 0.0) {

      glfwPollEvents();
    } else {

      glfwWaitEventsTimeout(delay_time);
    }
  }

  if (!sigisemptyset(&sigmask)) {

    for (std::vector<std::pair<int, std::function<int(int)>>>::iterator
             signalhandler = _SignalHandlers.begin();
         signalhandler != _SignalHandlers.end(); ++signalhandler) {

      if (sigismember(&sigmask, signalhandler->first)) {

        ret = signalhandler->second(signalhandler->first);
      }
    }
  }

  return ret;
}
