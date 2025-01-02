/**
 *  @file   WindowManager.h
 *  @brief  Window Manager Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef WINDOWMANAGER_H_
#define WINDOWMANAGER_H_

#include <list>

#include <functional>

#include "Image.h"

#include "ManagedWindow.h"

#include "WindowEvents.h"

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

class WindowManager {

public:
  WindowManager();

  ~WindowManager();

  WindowManager(int argc, char **argv);

  GLFWwindow *GetFileDescriptor();

  WindowEvent _event;

  int EventHandler(void);

  ManagedWindow *
  CreateWindow(int xpos, int ypos, int width, int height,
               std::function<WindowEvents(WindowEvent *)> handler = nullptr,
               Image *background = nullptr, Image *icon = nullptr);

  int DestroyWindow(ManagedWindow *mwindow);

private:
  int _argc;

  char **_argv;

  int _init(int argc, char *argv[]);

  GLFWwindow *xwindow = nullptr;

  std::list<ManagedWindow *> _mwindows;

  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

  static void MouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void CursorPositionCallback(GLFWwindow *window, double x, double y);
};

inline GLFWwindow *WindowManager::GetFileDescriptor() { return xwindow; }
#endif // End of WINDOWMANAGER_H_
