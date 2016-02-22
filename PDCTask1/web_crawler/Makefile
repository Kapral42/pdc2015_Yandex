prog := crawler
prog_objs := parser.o downloader.o main.o 

CXX := g++
CXXFLAGS := -g -std=c++11 -Wall -O2
LDFLAGS := -pthread -lcurl

.PHONY: all clean

all: $(prog)

$(prog): $(prog_objs)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

parser.o: parser.cpp
downloader.o: downloader.cpp
main.o: main.cpp

clean:
	@rm -rf *.o $(prog)
