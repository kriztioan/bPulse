/**
 *  @file   WindowManager.cpp
 *  @brief  Window Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-12-29
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "WindowManager.h"
#include "ManagedWindow.h"

WindowManager::WindowManager() { _init(0, nullptr); }

WindowManager::WindowManager(int argc, char **argv) { _init(argc, argv); }

WindowManager::~WindowManager() {

  std::list<ManagedWindow *>::iterator mwindow = _mwindows.begin();

  while (mwindow != _mwindows.end()) {

    delete *mwindow;

    mwindow = _mwindows.erase(mwindow);
  }

  glfwTerminate();
}

int WindowManager::_init(int argc, char **argv) {

  _argc = argc;

  _argv = argv;

  if (!glfwInit()) {

    return (1);
  }

  return (0);
}

void WindowManager::CursorPositionCallback(GLFWwindow *window, double x,
                                           double y) {

  WindowManager *self = (WindowManager *)glfwGetWindowUserPointer(window);

  ManagedWindow *mwindow = self->_mwindows.front();

  const static float radius_squared = powf((float)mwindow->xheight / 2, 2);

  if ((powf(x - (float)mwindow->xwidth / 4, 2) +
       powf(y - (float)mwindow->xheight / 4, 2)) <= radius_squared) {

    if (mwindow->_mouse_down) {

      mwindow->xx += x - mwindow->mouse_x;

      mwindow->xy += y - mwindow->mouse_y;

      glfwSetWindowPos(window, mwindow->xx, mwindow->xy);
    }
  }
}

void WindowManager::MouseButtonCallback(GLFWwindow *window, int button,
                                        int action, int mods) {

  WindowManager *self = (WindowManager *)glfwGetWindowUserPointer(window);

  ManagedWindow *mwindow = self->_mwindows.front();

  if (button == GLFW_MOUSE_BUTTON_LEFT) {

    if (action == GLFW_PRESS) {

      glfwGetCursorPos(window, &mwindow->mouse_x, &mwindow->mouse_y);

      mwindow->_mouse_down = true;
    } else if (action == GLFW_RELEASE) {

      if (mwindow->EventHandler) {

        self->_event.x = mwindow->xx;

        self->_event.y = mwindow->xy;

        self->_event.type = WindowEvents::Move;

        mwindow->EventHandler(&self->_event);
      }

      mwindow->_mouse_down = false;
    }
  }
}

void WindowManager::KeyCallback(GLFWwindow *window, int key, int scancode,
                                int action, int mods) {

  WindowManager *self = (WindowManager *)glfwGetWindowUserPointer(window);

  std::list<ManagedWindow *>::iterator mwindow = self->_mwindows.begin();

  while (mwindow != self->_mwindows.end()) {

    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) &&
        action == GLFW_RELEASE) {

      self->_event.type = WindowEvents::Destroy;

      (*mwindow)->EventHandler(&self->_event);
    }

    ++mwindow;
  }
}

int WindowManager::EventHandler() { return 0; }

int WindowManager::DestroyWindow(ManagedWindow *mwindow) {

  delete mwindow;

  _mwindows.remove(mwindow);

  mwindow = nullptr;

  return 0;
}

ManagedWindow *
WindowManager::CreateWindow(int xpos, int ypos, int width, int height,
                            std::function<WindowEvents(WindowEvent *)> handler,
                            Image *background, Image *icon) {

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

  glfwWindowHint(GLFW_SAMPLES, 4);

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_TRUE);

  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

  glfwWindowHint(GLFW_POSITION_X, xpos);

  glfwWindowHint(GLFW_POSITION_Y, ypos);

  ManagedWindow *mwindow = new ManagedWindow();

  mwindow->xx = xpos;

  mwindow->xy = ypos;

  mwindow->xwidth = width;

  mwindow->xheight = height;

  mwindow->EventHandler = handler;

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();

  float xscale, yscale;

  glfwGetMonitorContentScale(monitor, &xscale, &yscale);

  xwindow = mwindow->xwindow =
      glfwCreateWindow(width / xscale, height / yscale, "bPulse 2", NULL, NULL);

  if (!xwindow) {

    glfwTerminate();

    return nullptr;
  }

  glfwSetWindowPos(xwindow, xpos, ypos);

  GLFWimage xicon = {
      .width = icon->width, .height = icon->height, .pixels = icon->data};

  glfwSetWindowIcon(xwindow, 1, &xicon);

  mwindow->xcursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

  glfwMakeContextCurrent(xwindow);

  glewInit();

  glfwSwapInterval(0);

  glfwGetFramebufferSize(xwindow, &width, &height);

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  glOrtho(0.0, width, height, 0.0, 0.0, 1.0);

  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();

  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_TEST);

  glEnableClientState(GL_VERTEX_ARRAY);

  glEnable(GL_POLYGON_SMOOTH);

  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  glEnable(GL_LINE_SMOOTH);

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glEnable(GL_TEXTURE_2D);

  if (background) {

    glGenTextures(1, &mwindow->xbackground);

    glBindTexture(GL_TEXTURE_2D, mwindow->xbackground);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, background->width,
                 background->height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 background->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glfwSetWindowUserPointer(xwindow, this);

  glfwSetKeyCallback(xwindow, KeyCallback);

  glfwSetCursorPosCallback(xwindow, CursorPositionCallback);

  glfwSetMouseButtonCallback(xwindow, MouseButtonCallback);

  _mwindows.push_back(std::move(mwindow));

  return mwindow;
}
