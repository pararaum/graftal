#include <SDL2/SDL.h>
#include <iostream>
#include <cassert>
#include <boost/format.hpp>
#include <list>
#include <map>
#include "graftal.hh"
#include "displaygraf.hh"

#define WINWIDTH 1000
#define WINHEIGHT 800

class GfxGraftalIterator : public DisplayGraftal {
protected:
  GraftalIterator grafiter;
  std::list<GraftalIterator> graftalstack;
protected:

  void next_gen() {
    graftalstack.push_front(grafiter);
    grafiter.next();
  }

public:
  GfxGraftalIterator(SDL_Renderer *ren, const GraftalIterator::GraftalType &init, const GraftalIterator::RulesType rules_)
    : DisplayGraftal(ren, WINWIDTH/2, WINHEIGHT-25), grafiter(init, rules_) {}

  void handle_events(void) {
    SDL_Event event;
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    draw(grafiter.graftal);
    SDL_RenderPresent(ren);
    while(SDL_WaitEvent(&event) == 1) {
      switch(event.type) {
      case SDL_KEYDOWN:
	if(event.key.keysym.sym == SDLK_h) { }
	break;
      case SDL_QUIT:
	return;
      case SDL_MOUSEBUTTONDOWN:
	if(event.button.button == SDL_BUTTON_RIGHT) {
	  if(!graftalstack.empty()) {
	    grafiter = graftalstack.front();
	    graftalstack.pop_front();
	  }
	} else {
	  next_gen();
	}
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);
	draw(grafiter.graftal);
	SDL_RenderPresent(ren);
	break;
      default:
	//Ignore
	break;
      }
    }
  }
};

int main() {
  int ret = 0;
  std::list<std::pair<std::string,std::string> > rules {
    { "1111111", "1111111" },
    { "0", "1[0]1(0)0" },
      { "1", "11" }
    // { "11*1010001", "1[1](1)" },
    // { "1010001", "101001*1" },
    //   { "1010", "101[1]0" },
    // 	{ "00001", "0000(1)1" },
    // // { "101", "10(0)1" },
    // // { "010", "00[1]1" },
    // // { "*00", "*0" },
    // // { "*01", "*0" },
    // { "11", "101" },
    // { "01", "001" },
    // { "1", "11" },
  };

  if (SDL_Init(SDL_INIT_VIDEO) != 0){
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return -1;
  }
  SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, WINWIDTH, WINHEIGHT, SDL_WINDOW_SHOWN);
  if (win == nullptr){
    std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    ret = 1;
  } else {
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr){
      std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
      ret = 2;
    } else {
      std::cout << "Graftal BEGIN\n";
      GfxGraftalIterator graftal(ren, "0", rules);
      graftal.handle_events();
      std::cout << "Graftal END\n";
    }
    if(ren) SDL_DestroyRenderer(ren);
  }
  if(win) SDL_DestroyWindow(win);
  SDL_Quit();
  return ret;
}
