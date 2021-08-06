/**
 *  @file   WindowEvents.h
 *  @brief  Window Event Definitions
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef WINDOWEVENTS_H_
#define WINDOWEVENTS_H_

enum class WindowEvents { Zero = 0, Ignore = 1, Move = 2, Destroy = 3 };

typedef struct {
  WindowEvents type;
  int x;
  int y;
} WindowEvent;
#endif // End of WINDOWEVENTS_H_
