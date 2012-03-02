CXX=g++
CPPFLAGS=-g -Wall
LIBS=-lpthread

SERVER_OBJECTS=server.o util.o sorted_set.o commands.o hash_map.o skiplist.o 
UNIT_TEST_OBJECTS=sorted_set.o skiplist.o hash_map.o
CLIENT_OBJECTS=util.o commands.o

.PHONY: all clean rebuild run_tests

all: server unit_test client

server: run_server.cpp $(SERVER_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
	make run_tests
unit_test: unit_test.cpp $(UNIT_TEST_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
client: client.cpp $(CLIENT_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^

%.o: %.cpp %.h 
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf *.o server unit_test client *.dSYM
rebuild:
	make clean
	make

run_tests:
	@echo "[Start Unit Tests] No news is good newes"
	make unit_test
	./unit_test 
