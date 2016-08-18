#ifndef __GRAFTAL_HH__2016_
#define __GRAFTAL_HH__2016_
#include <string>
#include <list>

class GraftalIterator {
public:
  typedef std::string GraftalType;
  typedef std::list< std::pair<GraftalType,GraftalType> > RulesType;
public:
  GraftalType graftal;
public:
  RulesType rules;

  GraftalIterator(const std::string &init) : graftal(init) {}
  GraftalIterator(const std::string &init, const RulesType rules_) : graftal(init), rules(rules_) {}

  GraftalIterator &next();
};

#endif
