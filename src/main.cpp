#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_Image.h>
#include <SDL2/SDL_ttf.h>

#include "res_path.h"
#include "cleanup.h"


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

void logSDLError(std::ostream &os, const std::string &msg) {
  os << msg << " error: " << SDL_GetError() << std::endl;
}

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren) {

  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr){
    logSDLError(std::cout, "LoadTexture");
  }
  
  return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h) {
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;

  SDL_RenderCopy(ren, tex, NULL, &dst);
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  renderTexture(tex, ren, x, y, w, h);
}

SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
                        SDL_Color color, int fontSize, SDL_Renderer *ren)
{
  //Open the font
  TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
  if (font == nullptr){
    logSDLError(std::cout, "TTF_OpenFont");
    return nullptr;
  }
  //We need to first render to a surface as that's what TTF_RenderText
  //returns, then load that surface into a texture
  SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
  if (surf == nullptr){
    TTF_CloseFont(font);
    logSDLError(std::cout, "TTF_RenderText");
    return nullptr;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surf);
  if (texture == nullptr){
    logSDLError(std::cout, "CreateTexture");
  }
  //Clean up the surface and font
  SDL_FreeSurface(surf);
  TTF_CloseFont(font);
  return texture;
}

/*
 * Lesson 0: Test to make sure SDL is setup properly
 */
int main(int, char**){

  SDL_Event e;
  bool quit = false;
  struct timespec ts;

  if (SDL_Init(SDL_INIT_VIDEO) != 0){
    logSDLError(std::cout, "SDL_Init");
    return 1;
  }

  if (TTF_Init() != 0){
    logSDLError(std::cout, "TTF_Init");
    SDL_Quit();
    return 1;
  }

  SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (win == nullptr){
    logSDLError(std::cout, "CreateWin");
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (ren == nullptr){
    logSDLError(std::cout, "CreateRen");
    cleanup(win);
    SDL_Quit();
    return 1;
  }

  const std::string resPath = getResourcePath();
  //We'll render the string "TTF fonts are cool!" in white
  //Color is in RGBA format
  SDL_Color color = { 255, 255, 255, 255 };
  SDL_Texture *image;

  while(!quit) {

    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){
        quit = true;
      }
      if (e.type == SDL_KEYDOWN){
        quit = true;
      }
      if (e.type == SDL_MOUSEBUTTONDOWN){
        quit = true;
      }
    }

    SDL_RenderClear(ren);

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;

#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif

    time_t t = ts.tv_sec;
    struct tm lt;
    (void) localtime_r(&t, &lt);

    char timeStr[256];
    char format[] = "%r";
    if (strftime(timeStr, sizeof(timeStr), format, &lt) == 0) {
      return 1;
    }

    image = renderText(timeStr, resPath + "sample.ttf",
                                  color, 64, ren);
    if (image == nullptr){
      cleanup(ren, win);
      TTF_Quit();
      SDL_Quit();
      return 1;
    }

    //Get the texture w/h so we can center it in the screen
    int iW, iH;
    SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
    int x = SCREEN_WIDTH / 2 - iW / 2;
    int y = SCREEN_HEIGHT / 2 - iH / 2;
    
    //Note: This is within the program's main loop
    SDL_RenderClear(ren);
    //We can draw our message as we do any other texture, since it's been
    //rendered to a texture
    renderTexture(image, ren, x, y);
    SDL_RenderPresent(ren);
    
    SDL_Delay(1000);
  }


  cleanup(image, ren, win);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
