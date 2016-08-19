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

  /*! \brief draw the graftal using the renderer
   *
   * Draw the given graftal string using the stored render
   * context. TODO: Make it more configurable...
   */
  void draw(const std::string &graftal);
};

#endif
