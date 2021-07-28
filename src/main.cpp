/**
 *  @file   main.cpp
 *  @brief  bPulse
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "config.h"

#include "Image.h"

#include "ApplicationManager.h"
#include "ManagedWindow.h"
#include "ProcManager.h"
#include "SettingsManager.h"
#include "ThemeManager.h"
#include "WindowManager.h"

#include <functional>

#include <iostream>

int SignalHandler(int sig);

int CallbackHandler();

int HandleIO();

int HandleEth();

int HandleMem();

int HandleDisk();

int HandleCPU();

int HandleDate();

int HandleTime();

int HandleUser();

WindowEvents EventHandler(WindowEvent *e);

ApplicationManager *amanager = nullptr;

SettingsManager *smanager = nullptr;

ProcManager *pmanager = nullptr;

WindowManager *wmanager = nullptr;

ManagedWindow *mwindow = nullptr;

int timeout;

bool animate = false;

std::string font;

int main(int argc, char *argv[], char **envp) {

  smanager = new SettingsManager(argc, argv);

  smanager->LoadSettingsFromFile(SETTINGS_FILE);

  amanager = new ApplicationManager(argc, argv);

  std::function<int(int)> shandler = SignalHandler;

  amanager->RegisterSignalHandler(SIGINT, shandler);

  amanager->RegisterSignalHandler(SIGQUIT, shandler);

  amanager->RegisterSignalHandler(SIGTERM, shandler);

  amanager->RegisterSignalHandler(SIGHUP, shandler);

  wmanager = new WindowManager(argc, argv);

  amanager->RegisterEventHandler(
      wmanager->GetFileDescriptor(),
      std::bind(&WindowManager::EventHandler, wmanager));

  pmanager = new ProcManager;

  pmanager->SetDisk(smanager->GetOptionForKey("disk").c_str());

  pmanager->SetCPU(smanager->GetOptionForKey("cpu").c_str());

  pmanager->SetEth(smanager->GetOptionForKey("eth").c_str());

  pmanager->SetIO(smanager->GetOptionForKey("io").c_str());

  pmanager->SetProcMask(ProcManager::Masks::CPU | ProcManager::Masks::Mem |
                        ProcManager::Masks::Disk | ProcManager::Masks::Eth |
                        ProcManager::Masks::IO | ProcManager::Masks::Time |
                        ProcManager::Masks::Users);

  pmanager->Probe(); // initialize!

  std::function<int(void)> chandler = CallbackHandler;

  amanager->RegisterCallback(chandler);

  ThemeManager *tmanager = new ThemeManager();

  tmanager->LoadThemeFromFile(smanager->GetOptionForKey("theme").c_str());

  Image *background =
            new Image(tmanager->GetOptionForKey("background").c_str()),
        *icon = new Image(tmanager->GetOptionForKey("icon").c_str());

  std::function<WindowEvents(WindowEvent *)> ehandler = EventHandler;

  mwindow = wmanager->CreateWindow(
      atoi(smanager->GetOptionForKey("xpos").c_str()),
      atoi(smanager->GetOptionForKey("ypos").c_str()), background->width,
      background->height, ehandler, background, icon);

  mwindow->SetOpacity(atof(tmanager->GetOptionForKey("opacity").c_str()));

  delete background;

  delete icon;

  timeout = atoi(smanager->GetOptionForKey("timeout").c_str());

  font = tmanager->GetOptionForKey("font");

  amanager->SetTimeout(1000 / FRAME_RATE);

  amanager->RunLoop();

  smanager->WriteSettingsToFile();

  delete pmanager;

  delete smanager;

  delete tmanager;

  delete wmanager;

  delete amanager;

  return 0;
}

int SignalHandler(int sig) {

  std::cout << "Program received a signal (" << sig << "), quiting."
            << std::endl;

  return sig;
}

int CallbackHandler() {

  static unsigned long timer = 0;

  if (animate) {

    amanager->SetTimeout(1000 / FRAME_RATE);

    if (timer % (timeout / (1000 / FRAME_RATE)) == 0) {

      pmanager->Probe();
    }

    ++timer;
  } else {

    amanager->SetTimeout(1000 / 2);

    pmanager->Probe();
  }

  animate = false;

  HandleIO();

  HandleEth();

  HandleMem();

  HandleDisk();

  HandleCPU();

  HandleUser();

  HandleDate();

  HandleTime();

  mwindow->Sync();

  return 0;
}

WindowEvents EventHandler(WindowEvent *e) {

  switch (e->type) {

  case WindowEvents::Move:

    smanager->SetOptionForKey("xpos", e->x);

    smanager->SetOptionForKey("ypos", e->y);

    break;

  case WindowEvents::Destroy:

    amanager->TerminateLoop();

    break;

  default:

    // printf("Unhandled!!!\n");
    break;
  };

  return WindowEvents::Zero;
}

int HandleCPU() {

  static float cpu[3] = {0, 0, 0};

  float cpu_in[3] = {pmanager->cpu.sys, pmanager->cpu.user, pmanager->cpu.nice},
        sum = 0;

  for (int i = 0; i < 3; i++) {

    if (cpu_in[i] > 0.05 || cpu[i] > 0.05) {

      if (cpu_in[i] > 1.05 * cpu[i]) {

        if (cpu[i] < 0.01) {

          cpu[i] = 0.5 * cpu_in[i];
        }

        cpu[i] *= 1.2;

        animate = true;
      } else if (cpu_in[i] < 0.9 * cpu[i]) {

        cpu[i] *= 0.8;

        animate = true;
      }
    }

    sum += cpu[i];
  }

  if (sum > 1) {

    for (int i = 0; i < 3; i++) {

      cpu[i] /= sum;
    }

    sum = 1.0;
  }

  if (animate) {

    if (sum <= 0.01) {

      return 0;
    }

    mwindow->DrawCircle(CEN_X, CEN_Y, R0, R1 * sum, "rgba:14/07/68/ff");

    sum -= cpu[NICE];

    if (sum <= 0.01) {

      return 0;
    }

    mwindow->DrawCircle(CEN_X, CEN_Y, R0, R1 * sum, "rgba:56/52/9e/ff");

    sum -= cpu[USER];

    if (sum <= 0.01) {

      return 0;
    }

    mwindow->DrawCircle(CEN_X, CEN_Y, R0, R1 * sum, "rgba:af/ae/c4/ff");
  }

  return 0;
}

int HandleDate() {

  static char day[3], month[4];

  sprintf(day, "%02d", pmanager->time.tm_mday);

  strftime(month, 4, "%b", &(pmanager->time));

  mwindow->DrawText(CEN_X + R2, CEN_Y, day, font.c_str(), 10,
                    "rgba:00/00/ff/ff");

  mwindow->DrawText(CEN_X - R3, CEN_Y, month, font.c_str(), 10,
                    "rgba:00/00/ff/ff");

  return 0;
}

int HandleTime() {

  mwindow->DrawLine(
      CEN_X, CEN_Y,
      CEN_X + (0.75 * R3 *
               cos(2. * M_PI + M_PI / 2. -
                   2. * 2. * M_PI *
                       (pmanager->time.tm_hour + pmanager->time.tm_min / 60.) /
                       24.)),
      CEN_Y - (0.75 * R3 *
               sin(2. * M_PI + M_PI / 2. -
                   2. * 2. * M_PI *
                       (pmanager->time.tm_hour + pmanager->time.tm_min / 60.) /
                       24.)),
      3, "rgba:fc/ff/42/88");

  mwindow->DrawLine(CEN_X, CEN_Y,
                    CEN_X + (0.95 * R3 *
                             cos(2. * M_PI + M_PI / 2. -
                                 2. * M_PI * pmanager->time.tm_min / 60.)),
                    CEN_Y - (0.95 * R3 *
                             sin(2. * M_PI + M_PI / 2. -
                                 2. * M_PI * pmanager->time.tm_min / 60.)),
                    2, "rgba:fc/ff/42/88");

  mwindow->DrawLine(CEN_X - (R3 * 0.25 *
                             cos(2. * M_PI + M_PI / 2. -
                                 2. * M_PI * pmanager->time.tm_sec / 60.)),
                    CEN_Y + (R3 * 0.25 *
                             sin(2. * M_PI + M_PI / 2. -
                                 2. * M_PI * pmanager->time.tm_sec / 60.)),
                    CEN_X + (R3 * cos(2. * M_PI + M_PI / 2. -
                                      2. * M_PI * pmanager->time.tm_sec / 60.)),
                    CEN_Y - (R3 * sin(2. * M_PI + M_PI / 2. -
                                      2. * M_PI * pmanager->time.tm_sec / 60.)),
                    1, "rgba:ff/00/00/ff");

  return 0;
}

int HandleIO() {

  static double io[2] = {0, 0};

  double io_in[2] = {1000.0 * pmanager->io.read / timeout,
                     1000.0 * pmanager->io.write / timeout};

  static float steps[4] = {1, 1024, 100, 1024 / 100};

  for (int i = 0; i < 2; i++) {

    int j = 1;

    while (io_in[i] > 90) {

      io_in[i] /= steps[j++];
    }
  }

  for (int i = 0; i < 2; i++) {

    if (io_in[i] > 0.05 || io[i] > 0.05) {

      if (io_in[i] > 1.05 * io[i]) {

        if (io[i] < 0.01) {

          io[i] = 0.5 * io_in[i];
        }

        io[i] *= 1.2;

        animate = true;
      } else if (io_in[i] < 0.9 * io[i]) {

        io[i] *= 0.8;

        animate = true;
      }
    }
  }

  for (int i = 0; i < 2; i++) {

    if (io[i] > 90) {

      io[i] = 90;
    }
  }

  if (animate) {

    if (io[IN] > 1) {

      mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 180, 180 + io[IN],
                       "rgba:ff/2c/1c/ff");
    }

    if (io[OUT] > 1) {
      mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 360 - io[OUT], 360,
                       "rgba:66/ff/4f/ff");
    }
  }

  return 0;
}

int HandleEth() {

  static double eth[2] = {0, 0};

  double eth_in[2] = {1000.0 * pmanager->eth.sent / timeout,
                      1000.0 * pmanager->eth.received / timeout};

  static float steps[4] = {1, 1024, 100, 1024 / 100};

  for (int i = 0; i < 2; i++) {

    int j = 1;

    while (eth_in[i] > 90) {

      eth_in[i] /= steps[j++];
    }
  }

  for (int i = 0; i < 2; i++) {

    if (eth_in[i] > 0.05 || eth[i] > 0.05) {

      if (eth_in[i] > 1.05 * eth[i]) {

        if (eth[i] < 0.01) {

          eth[i] = 0.5 * eth_in[i];

          animate = true;
        }

        eth[i] *= 1.2;
      } else if (eth_in[i] < 0.9 * eth[i]) {

        eth[i] *= 0.8;

        animate = true;
      }
    }
  }

  for (int i = 0; i < 2; i++) {

    if (eth[i] > 90) {

      eth[i] = 90;
    }
  }

  if (animate) {

    if (eth[SENT] > 1) {

      mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 0, eth[SENT], "rgba:11/00/82/ff");
    }

    if (eth[RECV] > 1) {

      mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 180 - eth[RECV], 180,
                       "rgba:39/9c/c4/ff");
    }
  }

  return 0;
}

int HandleMem() {

  static float free = 0, buffer = 0, shared = 0, unavailable = 0, sum;

  if (pmanager->memory.freeram > 1.05 * free) {

    if (free < 1) {

      free = 0.5 * pmanager->memory.freeram;
    }

    free *= 1.2;

    animate = true;
  } else if (pmanager->memory.freeram < 0.95 * free) {

    free *= 0.95;

    animate = true;
  }

  if (pmanager->memory.bufferram > 1.05 * buffer) {

    if (buffer < 1) {

      buffer = 0.5 * pmanager->memory.bufferram;
    }

    buffer *= 1.2;

    animate = true;
  } else if (pmanager->memory.bufferram < 0.95 * buffer) {

    buffer *= 0.95;

    animate = true;
  }

  if (pmanager->memory.sharedram > 1.05 * shared) {

    if (shared < 1) {

      shared = 0.5 * pmanager->memory.sharedram;
    }

    shared *= 1.2;

    animate = true;
  } else if (pmanager->memory.sharedram < 0.95 * shared) {

    shared *= 0.95;

    animate = true;
  }

  unsigned long kernelram =
                    pmanager->memory.totalram - pmanager->memory.freeram -
                    pmanager->memory.bufferram - pmanager->memory.sharedram,
                totalram = pmanager->memory.totalram + kernelram;

  if (kernelram > 1.05 * unavailable) {

    if (unavailable < 1) {

      unavailable = 0.5 * kernelram;
    }

    unavailable *= 1.2;

    animate = true;
  } else if (kernelram < 0.95 * unavailable) {

    unavailable *= 0.95;

    animate = true;
  }

  sum = free + buffer + shared + unavailable;

  if (sum > totalram) {

    free = totalram * free / sum;

    buffer = totalram * buffer / sum;

    shared = totalram * shared / sum;

    unavailable = totalram * unavailable / sum;
  }

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, 0, 180 * free / totalram,
                   "rgba:ff/00/00/55");

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, 180 * free / totalram,
                   180 * (free + buffer) / totalram, "rgba:00/ff/00/ff");

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, 180 * (free + buffer) / totalram,
                   180 * (free + buffer + shared) / totalram,
                   "rgba:00/00/ff/ff");

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2,
                   180 * (free + buffer + shared) / totalram, 180,
                   "rgba:ff/ff/00/ff");

  return 0;
}

int HandleDisk() {

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, 180,
                   180 + static_cast<float>(180 * pmanager->disk.f_bfree) /
                             pmanager->disk.f_blocks,
                   "rgba:bd/56/90/ff");

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2,
                   180 + static_cast<float>(180 * pmanager->disk.f_bfree) /
                             pmanager->disk.f_blocks,
                   360, "rgba:a2/2e/d5/ff");

  return 0;
}

int HandleUser() {

    if (!pmanager->users.front().empty()) {
      mwindow->DrawText(CEN_X, CEN_Y - 4, pmanager->users.front().c_str(),
                        font.c_str(), 10, "rgba:dd/dd/dd/ff",
                        TEXT::ALIGN::CENTER);

      mwindow->DrawText(CEN_X, CEN_Y + 24,
                        std::to_string(pmanager->users.size()).c_str(),
                        font.c_str(), 10, "rgba:dd/dd/dd/ff",
                        TEXT::ALIGN::LEFT);
    }

  return 0;
}
