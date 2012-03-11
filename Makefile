CXX=g++
CPPFLAGS=-g -Wall
LIBS=-lpthread

SERVER_OBJECTS=server.o util.o sorted_set.o commands.o hash_map.o skiplist.o lockable.o
UNIT_TEST_OBJECTS=sorted_set.o skiplist.o hash_map.o lockable.o
CLIENT_OBJECTS=util.o commands.o

.PHONY: all clean rebuild run_tests pack

all: server unit_test client

server: run_server.cpp $(SERVER_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
	#make run_tests
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
pack:
	rm -rf sorted_set.tar.gz
	tar cvfz sorted_set.tar.gz *.h *.cpp Makefile README.md DESC.txt demo.sh

run_tests:
	@echo "[Start Unit Tests] No news is good newes"
	make unit_test
	./unit_test 
