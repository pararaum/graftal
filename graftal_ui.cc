#include <stdexcept>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include <cassert>
#include <thread>
#include "graftaleditor.hh"
#include "graftal.hh"
#include "displaygraf.hh"

#define WINWIDTH 1000
#define WINHEIGHT 800
#define ALLOCATED_EVENTS (EVENT_MAXIMUM + 1)

enum MyEvents {
  EVENT_NOOP,
  EVENT_REDRAW,
  EVENT_MAXIMUM
};

Uint32 minimum_event;

struct InitError: public std::exception {
  std::string msg;
  InitError(const std::string &msg_) : msg(msg_) {}
  virtual const char *what() const noexcept(true) { return msg.c_str(); }
};


class SDL {
public:
  SDL(Uint32 flags = 0) {
    if (SDL_Init(flags) != 0) throw InitError(SDL_GetError());
  }
  virtual ~SDL() {
    SDL_Quit();
    std::cerr << "Destroying SDL.\n";
  }
};

template<typename X> class InitKeeper {
  X *ptr;
public:
  InitKeeper() : ptr(nullptr) {}
  virtual ~InitKeeper() {}
};

template<typename X> class Mutex {
  SDL_mutex *mutex;
  X data;

public:
  class Proxy {
    Mutex *parent;
  public:
    Proxy(Mutex *parent_) : parent(parent_) {
      if(SDL_LockMutex(parent->mutex) != 0) throw std::runtime_error("can not lock mutex");
    }
    ~Proxy() {
      //std::cout << "~Proxy for " << parent << std::endl;
      SDL_UnlockMutex(parent->mutex);
    }
    //X *operator->() { return parent->data; }
    const X &operator*() const { return parent->data; }
    X &operator*() { return parent->data; }
  };

  Mutex() {}
  Mutex(const X &x) : data(x) {
    mutex = SDL_CreateMutex();
    if(!mutex) throw std::runtime_error("can not create mutex");
  }
  virtual ~Mutex() {
    SDL_DestroyMutex(mutex);
  }
  Proxy get() {
    //std::cout << "get for " << this << std::endl;
    return Proxy(this);
  }
};

class SDLInterface {
  SDL_Window *win;
  SDL_Renderer *ren;
public:
  SDLInterface(int width, int height, Uint32 window_flags) {
    int ret = SDL_CreateWindowAndRenderer(width, height, window_flags, &win, &ren);
    if (ret != 0){
      throw std::runtime_error(std::string("SDL_CreateWindowAndRenderer Error: ") + SDL_GetError());
    }
  }
  ~SDLInterface() {
    std::cerr << "~SDLInterface() " << win << ' ' << ren << std::endl;
    assert(win);
    SDL_DestroyWindow(win);
    assert(ren);
    SDL_DestroyRenderer(ren);
  }
  //SDL_Window *operator->() { return win; }
  operator SDL_Window *() const { return win; }
  operator SDL_Renderer *() const { return ren; }
};

int useless_function(void *data) {
  assert(data == NULL);
  for(int i = 41; i > 0; --i) {
    SDL_Delay(1000);
    std::cout << "i = " << i << std::endl;
  }
  return 0;
}

/*! \brief push one of the user events
 *
 * Push one of the user events into the SDL queue.
 *
 * \param eventnum event number to push
 */
void push_user_event(MyEvents eventnum) {
  assert(eventnum < EVENT_MAXIMUM);
  SDL_Event event;
  event.type = eventnum + minimum_event;
  SDL_PushEvent(&event);
}

class ExtendedUserInterface : private UserInterface {
  Mutex<GraftalIterator> *grafiter;
public:
  ExtendedUserInterface(Mutex<GraftalIterator> *grafiter_) : UserInterface(), grafiter(grafiter_) {}
  virtual void new_graftal(const char *txt) {
    {
      auto graf = grafiter->get();
      (*graf).graftal = txt;
      std::cout << "New graftal set to: " << txt << std::endl;
    }
    push_user_event(EVENT_REDRAW);
  }
};

int flthreadfun(Mutex<GraftalIterator> &data) {
  ExtendedUserInterface *ui = new ExtendedUserInterface(&data);
  assert(ui);
  int ret = Fl::run();
  std::cout << "flthread: ret=" << ret << std::endl;
  SDL_Event event;
  event.type = SDL_QUIT;
  SDL_PushEvent(&event);
  return ret;
}

void draw_it(Mutex<SDLInterface&> &msdl, Mutex<GraftalIterator> &grafiter) {
  auto sdlint = msdl.get();
  SDL_Renderer *ren = static_cast<SDL_Renderer*>(*sdlint);
  std::cout << ren << std::endl;
  SDL_SetRenderDrawColor(ren, 0, 0, 0x5A, 255);
  SDL_RenderClear(ren);
  DisplayGraftal disp(*sdlint, WINWIDTH/2, WINHEIGHT-10);
  auto gf = grafiter.get();
  disp.draw((*gf).graftal);
  SDL_RenderPresent(ren);
}

void event_loop(Mutex<SDLInterface&> &msdl, Mutex<GraftalIterator> &grafiter) {
  bool running = true;
  int count = 0;
  draw_it(msdl, grafiter);
  do {
    SDL_Event event;
    while(SDL_PollEvent(&event) == 1) {
      switch(event.type) {
      case SDL_QUIT:
	running = false;
	break;
      case SDL_MOUSEBUTTONDOWN:
	std::cout << "Button!\n";
	break;
      default:
	if(event.type == EVENT_REDRAW + minimum_event) {
	  std::cout << "Redraw!\n";
	  draw_it(msdl, grafiter);
	}
	break;
      }
    }
    count++;
    SDL_Delay(145);
  } while(running);
}

int main(int argc, char **argv) {
  try {
    SDL sdl(SDL_INIT_VIDEO);
    minimum_event = SDL_RegisterEvents(ALLOCATED_EVENTS);
    if(minimum_event != ((Uint32)-1)) {
      SDLInterface wren(WINWIDTH, WINHEIGHT, SDL_WINDOW_SHOWN);
      Mutex<SDLInterface&> msdl(wren);
      Mutex<GraftalIterator> grafiter(GraftalIterator("1"));
      //SDL_SetWindowBordered(wren, SDL_FALSE);
      std::thread threadtry(useless_function, (void*)NULL);
      threadtry.detach();
      std::thread flthread(flthreadfun, std::ref(grafiter));
      flthread.detach();
      //Handle events in main thread; if this ends then the program ends.
      event_loop(msdl, grafiter);
    }
  }
  catch(const std::exception &excp) {
    std::cerr << "Exception: " << excp.what() << std::endl;
    return -1;
  }
  return 0;
}
