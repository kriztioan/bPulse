/**
 *  @file   SettingsManager.cpp
 *  @brief  Settings Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "SettingsManager.h"

SettingsManager::SettingsManager() {

    _init(0, NULL);
}

SettingsManager::SettingsManager(int argc, char *argv[]) {
    
    _init(argc, argv);
}

int SettingsManager::_init(int argc, char *argv[]) {

    return 0;
}

int SettingsManager::LoadSettingsFromFile(const char *path) {
    
    _SettingsFilePath = std::string(path);
    
    std::string key, value, *ptr;
    
    std::ifstream ifstr(_SettingsFilePath.c_str(), std::ios::in);

    if(ifstr.fail()) {

      return 1;
    }
	
    bool quotes = false;
    
	char character;
    
    while(ifstr.get(character).good()) {

		if(character == '#') {

			while(ifstr.get(character).good()) {

				if(character == '\n') {

					break;
                }
			}
		}
		else if(character == '$') {

            key.clear();
            
            ptr = &key;
            
			while(ifstr.get(character).good()) {

				if(character == '\n') {

                  _Options.insert(std::make_pair(key, value));
                
                   break;
                }
                else if(character == '\\') {

					while(ifstr.get(character).good()) {

						if(character == '\n') {

							break;
                        }
                    }
				}
                else if(character == '=' && !quotes) {
                    
                    value.clear();
                    
                    ptr = &value;
                }
                else if(character == '\"') {
                    
                    quotes = !quotes;
                }
				else if(character != ' ' && character != '\t') {

                    *ptr += character;
                }
			}
		}
	}

	ifstr.close();
    
    return 0;
}

int SettingsManager::WriteSettingsToFile() {

  std::ofstream ofstr(_SettingsFilePath.c_str(), std::ios::out);

  if(ofstr.fail()) {

    return 1;
  }

  ofstr << "#" << std::endl
	<< "# Configuration file for bPulse" << std::endl
	<< "#" << std::endl;

  for(std::map<std::string, std::string>::iterator option = _Options.begin(); option != _Options.end(); ++option) {

    if(option->second.find_first_not_of("0123456789.-") == std::string::npos) {

      ofstr << "$" << option->first << " = " << option->second << std::endl;

      continue;
    }

      ofstr << "$" << option->first << " = \"" << option->second << "\"" << std::endl;
  }

  ofstr.close();

  return 0;
}

std::string SettingsManager::GetOptionForKey(const char *key) {
    
    std::map<std::string, std::string>::iterator pair;
    
    pair = _Options.find(std::string(key));
    
    if(pair != _Options.end()) {

        return pair->second;
    }
    
    return std::string("");
}

int SettingsManager::SetOptionForKey(const char *key, int value) {

  std::map<std::string, std::string>::iterator it = _Options.find(key);

  if(it != _Options.end()) {

    _Options[key] = std::to_string(value);

    return 0;
  }

  _Options.insert(std::make_pair(key, std::to_string(value)));

  return 0;
}



