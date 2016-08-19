#include "displaygraf.hh"

void DisplayGraftal::draw(const std::string &graftal) {
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
