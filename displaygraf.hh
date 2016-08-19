#ifndef __DISPLAYGRAFTAL_HH_20160819__
#define __DISPLAYGRAFTAL_HH_20160819__
#include <SDL2/SDL.h>
#include <iostream>
#include <cassert>
#include <boost/format.hpp>
#include <list>
#include <map>

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
  DisplayGraftal(SDL_Renderer *ren_, int beginx, int beginy) : ren(ren_), state({-M_PI/2, 1, static_cast<double>(beginx), static_cast<double>(beginy)}) {}
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

#endif
