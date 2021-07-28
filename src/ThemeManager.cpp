/**
 *  @file   ThemeManager.cpp
 *  @brief  Theme Manager Class Implementation
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2021-07-27
 *  @note   BSD-3 licensed
 *
 ***********************************************/

#include "ThemeManager.h"

ThemeManager::ThemeManager() {

    _init(0, NULL);
}

ThemeManager::ThemeManager(int argc, char *argv[]) {
    
    _init(argc, argv);
}

int ThemeManager::_init(int argc, char *argv[]) {

    return 0;
}

int ThemeManager::LoadThemeFromFile(const char *path) {
    
    _ThemeFilePath = std::string(path);
    
    std::string key, value, *ptr;
    
	std::ifstream ifstr(_ThemeFilePath.c_str(), std::ios::in);

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

std::string ThemeManager::GetOptionForKey(const char *key) {
    
    std::map<std::string, std::string>::iterator pair;
    
    pair = _Options.find(std::string(key));
    
    if(pair != _Options.end()) {

        return pair->second;
    }
    
    return std::string("");
}


