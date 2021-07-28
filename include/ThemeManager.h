/**
 *  @file   ThemeManager.h
 *  @brief  Theme Manager Class Definition
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#ifndef THEMEMANAGER_H_
#define THEMEMANAGER_H_

#include <fstream>
#include <map>
#include <string>

class ThemeManager {

public:
  ThemeManager();

  ThemeManager(int argc, char *argv[]);

  int LoadThemeFromFile(const char *path);

  std::string GetOptionForKey(const char *key);

private:
  int _init(int argc, char *argv[]);

  std::string _ThemeFilePath;

  std::map<std::string, std::string> _Options;
};
#endif // End of THEMEMANAGER_H_
