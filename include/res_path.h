#ifndef RES_PATH_H
#define RES_PATH_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>

std::string 
getResourcePath(const std::string &subDir = "") 
{

  const char PATH_SEP = '/';
  static std::string baseRes;
  if (baseRes.empty()) {
    char *basePath = SDL_GetBasePath();
    if (basePath) {
      baseRes = basePath;
      SDL_free(basePath);
    }
    else {
      std::cerr << "Error getting resource path: " << SDL_GetError() << std::endl;
      return "";
    }

    size_t pos = baseRes.rfind("src");
    baseRes = baseRes.substr(0, pos) + "res" + PATH_SEP;
  }

  return subDir.empty() ? baseRes : baseRes + subDir + PATH_SEP;
}

#endif
