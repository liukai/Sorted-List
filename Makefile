CXX=g++
# TODO: NEED REFACTORING: what is cppflags and what is cxxflags
CPPFLAGS=-g -Wall
LIBS=-lpthread

SERVER_OBJECTS=server.o util.o sorted_set.o operations.o hash_map.o skiplist.o 
UNIT_TEST_OBJECTS=sorted_set.o skiplist.o hash_map.o
CLIENT_OBJECTS=util.o operations.o

.PHONY: all clean

all: server unit_test client

server: run_server.cpp $(SERVER_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
	# Run unit test after recompilation
	make unit_test
	./unit_test 
unit_test: unit_test.cpp $(UNIT_TEST_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
client: client.cpp $(CLIENT_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^

%.o: %.cpp %.h 
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf *.o server unit_test client *.dSYM
