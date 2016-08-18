#include <SDL2/SDL.h>
#include <iostream>
#include <cassert>
#include <boost/format.hpp>
#include <list>
#include <map>
#include "graftal.hh"

#define WINWIDTH 1000
#define WINHEIGHT 800

class DisplayGraftal {
protected:
  SDL_Renderer *ren;
  struct GrafState {
    double phi;
    double scale;
    double x;
    double y;
  } state;
  std::list<GrafState> sstack;

  void setgray(double g) {
    assert(g >= 0 && g <= 1);
    Uint8 c = static_cast<Uint8>(g * 255);
    SDL_SetRenderDrawColor(ren, c, c, c, 238);
  }
  void moveto(double x, double y) {
    state.x = x;
    state.y = y;
  }
  void lineto(double x, double y) {
    //std::cout << boost::format("%13.4g %13.4g %13.4g %13.4g\n") % state.x % state.y % x % y;
    int ret = SDL_RenderDrawLine(ren, state.x, state.y, x, y);
    if(ret != 0) {
      std::cerr << "Render error " << ret << ": " << SDL_GetError() << std::endl;
    }
    moveto(x, y);
  }
  void rlineto(double dx, double dy) {
    lineto(state.x + dx, state.y + dy);
  }

public:
  DisplayGraftal(SDL_Renderer *ren_) : ren(ren_), state({-M_PI/2, 1, WINWIDTH / 2, WINHEIGHT - 25}) {}
  virtual ~DisplayGraftal() {}

  void handle_events(void) {
    SDL_Event event;
    while(SDL_WaitEvent(&event) == 1) {
      switch(event.type) {
      case SDL_QUIT:
	return;
      case SDL_MOUSEBUTTONDOWN:
	std::cout << "Button!\n";
	break;
      default:
	//Ignore
	break;
      }
    }
  }

  void draw(const std::string &graftal) {
    const double len = 4;
    std::string::size_type pos = 0;
    GrafState oldstate(state);
    SDL_Rect rect;

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);
    //std::cerr << '"' << graftal << '"' << std::endl;
    while(pos < graftal.size()) {
      //std::cout << '\t' << graftal[pos] << std::endl;
      switch(graftal[pos]) {
      case '1':
	setgray(.442010101);
	rlineto(cos(state.phi) * len * state.scale, sin(state.phi) * len * state.scale);
	break;
      case '0':
	setgray(.8101441);
	rlineto(cos(state.phi) * len * state.scale, sin(state.phi) * len * state.scale);
	break;
      case '[':
	sstack.push_front(state);
	state.phi += 2*M_PI/360*28.4;
	state.scale *= 0.9363738282;
	break;
      case '(':
	sstack.push_front(state);
	state.phi -= 2*M_PI/360*28.4;
	state.scale *= 0.9363738282;
	break;
      case ')':
      case ']':
	assert(!sstack.empty());
	state = sstack.front();
	sstack.pop_front();
	break;
      case '*':
	rect.x = state.x - 2;
	rect.y = state.y - 2;
	rect.w = 5;
	rect.h = 5;
	SDL_SetRenderDrawColor(ren, 200, 10, 10, 181);
	SDL_RenderFillRect(ren, &rect);
	break;
      default:
	std::cerr << "Unknown char $" << std::hex << (int)graftal[pos] << std::endl;
      }
      pos += 1;
    }
    SDL_RenderPresent(ren);
    state = oldstate;
  }
};


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
    : DisplayGraftal(ren), grafiter(init, rules_) {}

  void handle_events(void) {
    SDL_Event event;
    draw(grafiter.graftal);
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
	draw(grafiter.graftal);
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
