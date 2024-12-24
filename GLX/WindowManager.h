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

#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>
#include <X11/keysymdef.h>

#include <GL/glx.h>

#include <Xm/MwmUtil.h>

#include "Image.h"

#include "ManagedWindow.h"

#include "WindowEvents.h"

class WindowManager {

public:
  WindowManager();

  ~WindowManager();

  WindowManager(int argc, char **argv);

  int GetFileDescriptor();

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

  int Image2XPixmap(Image *image, Pixmap *pixmap, Pixmap *mask);

  Display *_xdisplay;

  std::list<ManagedWindow *> _mwindows;
};

inline int WindowManager::GetFileDescriptor() {

  return ConnectionNumber(_xdisplay);
}
#endif // End of WINDOWMANAGER_H_
