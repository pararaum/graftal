#! /usr/bin/make -f

CXXFLAGS=-g -Wall -O2 -std=c++11
LIBS=-lSDL2
OBJS=graftal.o

all: graftaleditor.cc simple_graftal graftal_ui

simple_graftal: simple_graftal.o $(OBJS)
	$(CXX) -g -o $@ $< $(LIBS) $(OBJS)

graftal_ui: graftal_ui.o graftaleditor.o $(OBJS)
	$(CXX) -g -o $@ $+ $(LIBS) -lfltk

graftaleditor.cc: graftaleditor.fl
	fluid -c $<

.PHONY: clean
clean:
	rm simple_graftal graftal_ui *.o

