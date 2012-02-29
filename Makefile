CXX=g++
# TODO: NEED REFACTORING: what is cppflags and what is cxxflags
CPPFLAGS=-g -Wall
LIBS=-lpthread

SERVER_OBJECTS=sorted_set_server.o util.o set_manager.o
UNIT_TEST_OBJECTS=set_manager.o

.PHONY: all clean

all: server unit_test

server: sorted_set.cpp $(SERVER_OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
	# Run unit test after recompilation
	make unit_test
	./unit_test 
unit_test: unit_test.cpp
	$(CXX) $(CPPFLAGS) $(LIBS) -o $@ $^
$(BIN)/%.o: %.cpp %.h 
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf $(SERVER_OBJECTS) server unit_test *.dSYM
