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

time_t t;

struct tm *tm_s;

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
                        ProcManager::Masks::IO | ProcManager::Masks::Users);

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

  mwindow->SetFont(tmanager->GetOptionForKey("font"), 10);

  mwindow->SetAlwaysOnTop(
      atoi(smanager->GetOptionForKey("alwaysontop").c_str()));

  mwindow->SetOpacity(atof(tmanager->GetOptionForKey("opacity").c_str()));

  delete background;

  delete icon;

  timeout = atoi(smanager->GetOptionForKey("timeout").c_str());

  amanager->SetTimeout(1000 / FRAME_RATE);

  CallbackHandler();

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

  static unsigned long tick = 0;

  if (mwindow->IsLocked()) {

    ++tick;

    return 0;
  }

  if (tick++ % (timeout / (1000 / FRAME_RATE)) == 0) {

    pmanager->Probe();
  }

  t = time(NULL);

  tm_s = localtime(&t);

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

  static float cpu[3] = {0.0f, 0.0f, 0.0f};

  float cpu_in[3] = {pmanager->cpu.sys, pmanager->cpu.user, pmanager->cpu.nice},
        sum = 0.0f, r1 = 0.0;

  for (int i = 0; i < 3; i++) {

    cpu[i] = 0.9f * cpu[i] + 0.1f * cpu_in[i];

    sum += cpu[i];
  }

  if (sum > 0.0f) {

    mwindow->DrawCircle(CEN_X, CEN_Y, R0, r1, "rgba:14/07/68/ff");

    r1 = R1 * std::sqrt(sum);
  }

  sum -= cpu[NICE];
  if (sum > 0.0f) {

    r1 = R1 * std::sqrt(sum);

    mwindow->DrawCircle(CEN_X, CEN_Y, R0, r1, "rgba:56/52/9e/ff");
  }

  sum -= cpu[USER];
  if (sum > 0.0f) {

    r1 = R1 * std::sqrt(sum);

    mwindow->DrawCircle(CEN_X, CEN_Y, R0, r1, "rgba:af/ae/c4/ff");
  }

  return 0;
}

int HandleDate() {

  static char day[3], month[4];

  snprintf(day, 3, "%02d", tm_s->tm_mday);

  strftime(month, 4, "%b", tm_s);

  mwindow->DrawText(CEN_X + R2, CEN_Y, day, "rgba:00/00/ff/ff");

  mwindow->DrawText(CEN_X - R3, CEN_Y, month, "rgba:00/00/ff/ff");

  return 0;
}

int HandleTime() {

  mwindow->DrawLine(
      CEN_X, CEN_Y,
      CEN_X +
          (0.75 * R3 *
           cos(2. * M_PI + M_PI / 2. -
               2. * 2. * M_PI * (tm_s->tm_hour + tm_s->tm_min / 60.) / 24.)),
      CEN_Y -
          (0.75 * R3 *
           sin(2. * M_PI + M_PI / 2. -
               2. * 2. * M_PI * (tm_s->tm_hour + tm_s->tm_min / 60.) / 24.)),
      3, "rgba:fc/ff/42/88");

  mwindow->DrawLine(
      CEN_X, CEN_Y,
      CEN_X + (0.95 * R3 *
               cos(2. * M_PI + M_PI / 2. - 2. * M_PI * tm_s->tm_min / 60.)),
      CEN_Y - (0.95 * R3 *
               sin(2. * M_PI + M_PI / 2. - 2. * M_PI * tm_s->tm_min / 60.)),
      2, "rgba:fc/ff/42/88");

  mwindow->DrawLine(
      CEN_X - (R3 * 0.25 *
               cos(2. * M_PI + M_PI / 2. - 2. * M_PI * tm_s->tm_sec / 60.)),
      CEN_Y + (R3 * 0.25 *
               sin(2. * M_PI + M_PI / 2. - 2. * M_PI * tm_s->tm_sec / 60.)),
      CEN_X +
          (R3 * cos(2. * M_PI + M_PI / 2. - 2. * M_PI * tm_s->tm_sec / 60.)),
      CEN_Y -
          (R3 * sin(2. * M_PI + M_PI / 2. - 2. * M_PI * tm_s->tm_sec / 60.)),
      1, "rgba:ff/00/00/ff");

  return 0;
}

int HandleIO() {

  static double io[2] = {0, 0};

  double io_in[2] = {static_cast<double>(pmanager->io.read) / timeout,
                     static_cast<double>(pmanager->io.write) / timeout};

  for (int i = 0; i < 2; i++) {

    while (io_in[i] >= 1023.0) {

      io_in[i] /= 1024.0;
    }

    io[i] = 0.9 * io[i] + 0.1 * 90.0 * io_in[i] / 1024.0;
  }

  if (io[IN] > 0.0)
    mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 180, 180 + io[IN],
                     "rgba:ff/2c/1c/ff");

  if (io[OUT] > 0.0)
    mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 360 - io[OUT], 360,
                     "rgba:66/ff/4f/ff");

  return 0;
}

int HandleEth() {

  static double eth[2] = {0, 0};

  double eth_in[2] = {static_cast<double>(pmanager->eth.sent) /
                          static_cast<double>(timeout),
                      static_cast<double>(pmanager->eth.received) /
                          static_cast<double>(timeout)};

  for (int i = 0; i < 2; i++) {

    while (eth_in[i] >= 1023.0) {

      eth_in[i] /= 1024.0;
    }

    eth[i] = 0.9 * eth[i] + 0.1 * 90.0 * eth_in[i] / 1024.0;
  }

  if (eth[SENT] > 0.0)
    mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 0, eth[SENT], "rgba:11/00/82/ff");

  if (eth[RECV] > 0.0)
    mwindow->DrawArc(CEN_X, CEN_Y, R2, R3, 180 - eth[RECV], 180,
                     "rgba:39/9c/c4/ff");

  return 0;
}

int HandleMem() {

  static float free = 180.0f * pmanager->memory.freeram /
                      pmanager->memory.totalram,
               buffer = 180.0f * pmanager->memory.bufferram /
                        pmanager->memory.totalram,
               shared = 180.0f * pmanager->memory.sharedram /
                        pmanager->memory.totalram,
               kernel =
                   180.0f *
                   (pmanager->memory.totalram - pmanager->memory.freeram -
                    pmanager->memory.bufferram - pmanager->memory.sharedram) /
                   pmanager->memory.totalram;

  free = 0.9f * free +
         0.1f * 180.0f * pmanager->memory.freeram / pmanager->memory.totalram;

  buffer = 0.9f * buffer + 0.1f * 180.0f * pmanager->memory.bufferram /
                               pmanager->memory.totalram;

  shared = 0.9f * shared + 0.1f * 180.0f * pmanager->memory.sharedram /
                               pmanager->memory.totalram;

  kernel = 0.9f * kernel +
           0.1f * 180.0f *
               (pmanager->memory.totalram - pmanager->memory.freeram -
                pmanager->memory.bufferram - pmanager->memory.sharedram) /
               pmanager->memory.totalram;

  float val0 = 0.0f, val1 = free;

  if (val1 > 0.0f)
    mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, val0, val1, "rgba:ff/00/00/ff");

  val0 = val1;
  val1 += buffer;

  if (val1 > 0.0f)
    mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, val0, val1, "rgba:00/ff/00/ff");

  val0 = val1;
  val1 += shared;

  if (val1 > 0.0f)
    mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, val0, val1, "rgba:00/00/ff/ff");

  val0 = val1;
  val1 += kernel;

  if (val1 > 0.0f)
    mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, val0, val1, "rgba:ff/ff/00/ff");

  return 0;
}

int HandleDisk() {

  static float free = static_cast<float>(pmanager->disk.f_bfree) /
                      static_cast<float>(pmanager->disk.f_blocks);

  free = 0.9f * free + 0.1f * static_cast<float>(pmanager->disk.f_bfree) /
                           static_cast<float>(pmanager->disk.f_blocks);

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, 180, 180 + 180.0f * free,
                   "rgba:bd/56/90/ff");

  mwindow->DrawArc(CEN_X, CEN_Y, R1, R2, 180.0f + 180.0f * free, 360,
                   "rgba:a2/2e/d5/ff");

  return 0;
}

int HandleUser() {

  if (!pmanager->users.front().empty()) {
    mwindow->DrawText(CEN_X - 21, CEN_Y - 8, pmanager->users.front(),
                      "rgba:dd/dd/dd/ff", TEXT::ALIGN::CENTER);

    mwindow->DrawText(CEN_X, CEN_Y + 24, std::to_string(pmanager->users.size()),
                      "rgba:dd/dd/dd/ff", TEXT::ALIGN::LEFT);
  }

  return 0;
}
