/**
 *  @file   SettingsManager.h
 *  @brief  Settings Manager Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef SETTINGSMANAGER_H_
#define SETTINGSMANAGER_H_

#include <fstream>
#include <map>
#include <string>

class SettingsManager {

public:
  SettingsManager();

  SettingsManager(int argc, char *argv[]);

  int LoadSettingsFromFile(const char *path);

  int WriteSettingsToFile();

  std::string GetOptionForKey(const char *key);

  int SetOptionForKey(const char *key, int value);

private:
  int _init(int argc, char *argv[]);

  std::string _SettingsFilePath;

  std::map<std::string, std::string> _Options;
};
#endif // End of SETTINGSMANAGER_H_
