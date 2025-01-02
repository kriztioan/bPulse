/**
 *  @file   ApplicationManager.h
 *  @brief  Application Manager Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-12-29
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef APPLICATIONMANAGER_H_
#define APPLICATIONMANAGER_H_

#include <functional>
#include <utility>
#include <vector>

#include <signal.h>
#include <sys/time.h>

#include <GL/glew.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

class ApplicationManager {

public:
  ApplicationManager();

  ApplicationManager(int argc, char *argv[]);

  int RegisterCallback(std::function<int(void)> callback);

  int RegisterEventHandler(GLFWwindow *window,
                           std::function<int(void)> handler);

  int UnRegisterEventHandler(GLFWwindow *window);

  int RegisterSignalHandler(int signal, std::function<int(int)> handler);

  int UnRegisterSignalHandler(int signal);

  int SetTimeout(int msec);

  int RunLoop();

  int TerminateLoop();

private:
  int _init(int argc, char *argv[]);

  std::vector<std::function<int(void)>> _Callbacks;

  std::vector<std::pair<GLFWwindow *, std::function<int(void)>>> _EventHandlers;

  std::vector<std::pair<int, std::function<int(int)>>> _SignalHandlers;

  int _RefreshInterval;

  bool _finished = false;

  double timeout = 0;
};

inline int ApplicationManager::TerminateLoop() {

  _finished = true;

  return 0;
}

inline int ApplicationManager::SetTimeout(int msec) {

  timeout = (float)msec / 1000.0f;

  return 0;
}
#endif // End of APPLICATIONMANAGER_H_
