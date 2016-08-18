#include "graftal.hh"

GraftalIterator &GraftalIterator::next() {
  std::string news;
  const std::string old(graftal);
  unsigned long pos = 0;
  
  while(pos < old.size()) {
    for(auto i : rules) {
      if((i.first.size() <= old.size() - pos) && (old.substr(pos, i.first.size()) == i.first)) {
	news += i.second;
	pos += i.first.size();
	goto skip;
      }
    }
    news += old[pos];
    pos++;
  skip:
    ;;
  }
  std::swap(news, graftal);
    return *this;
}
