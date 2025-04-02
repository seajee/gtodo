CXX=g++
CXXFLAGS=-Wall -Wextra `pkg-config --cflags gtkmm-4.0`
LDFLAGS=`pkg-config --libs gtkmm-4.0`

all: gtodo

gtodo: main.cpp
	$(CXX) $(CXXFLAGS) -o gtodo main.cpp $(LDFLAGS)

clean:
	rm -rf gtodo
