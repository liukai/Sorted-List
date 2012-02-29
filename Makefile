CXX=g++
# TODO: NEED REFACTORING: what is cppflags and what is cxxflags
CPPFLAGS=-g -Wall
CXXFLAGS=-lpthread

SERVER_OBJECTS=sorted_set.cpp  sorted_set_server.o 

.PHONY: all clean

all: server

server: $(SERVER_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(BIN)/%.o: %.cpp %.h 
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf $(SERVER_OBJECTS) server
