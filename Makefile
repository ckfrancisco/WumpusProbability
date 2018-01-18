# Makefile

CC = c++
OBJECTS = Action.o Agent.o Location.o Orientation.o Percept.o WumpusWorld.o Search.o

PYTHON-DEF = -DPYTHON
PYTHON-OBJ = Action.o PyAgent.o Location.o Orientation.o Percept.o WumpusWorld.o

# Settings for MacOS
PYTHON-INC = -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
PYTHON-LIB = -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib -lpython2.7

# Settings for CentOS Linux
#PYTHON-INC = -I/usr/include/python2.7
#PYTHON-LIB = -L/usr/lib64 -lpython2.7

all: wumpsim

pywumpsim: wumpsim.h wumpsim.cc $(PYTHON-OBJ)
	$(CC) -g -o pywumpsim wumpsim.cc $(PYTHON-DEF) $(PYTHON-INC) $(PYTHON-OBJ) $(PYTHON-LIB)

wumpsim: wumpsim.h wumpsim.cc $(OBJECTS)
	$(CC) -g -o wumpsim wumpsim.cc $(OBJECTS)

Search.o: Search.h Search.cc
	$(CC) -g -c Search.cc

Action.o: Action.h Action.cc
	$(CC) -g -c Action.cc

Agent.o: Agent.h Agent.cc
	$(CC) -g -c Agent.cc

PyAgent.o: PyAgent.h PyAgent.cc
	$(CC) -g -c PyAgent.cc $(PYTHON-INC)

Location.o: Location.h Location.cc
	$(CC) -g -c Location.cc

Orientation.o: Orientation.h Orientation.cc
	$(CC) -g -c Orientation.cc

Percept.o: Percept.h Percept.cc
	$(CC) -g -c Percept.cc

WumpusWorld.o: WumpusWorld.h WumpusWorld.cc
	$(CC) -g -c WumpusWorld.cc

clean:
	rm -f *.o wumpsim pywumpsim
