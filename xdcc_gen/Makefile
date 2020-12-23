CXX=g++

LDFLAGS += -lactivemq-cpp
CXXFLAGS += -g $(LLVMFLAGS) -std=c++14 -lboost_system -lboost_filesystem

OBJ = \
	main.o \
	GenEcho.o \
	GenEgress.o \
	util.o \
	Gen.o \
	XdccFlow.o

TEST_OBJ = \
	run.o

EXEC = xdcc_gen

TEST_EXEC = test_code_gen
TEST_INCLUDE = \
	-I../build/apps/eop1/MA_v0.7_dist/MA_v0.7_src/include \
	-I/usr/local/include/activemq-cpp-3.9.5 \
	-I/usr/local/apr/include/apr-1

default: all

all: $(EXEC)

test: pre_test $(TEST_EXEC) run_test

$(EXEC): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(TEST_EXEC): $(TEST_OBJ)
	echo XXX $(TEST_INCLUDE)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

.PHONY : clean pre_test run_test

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@ -c $(TEST_INCLUDE)

clean:
	-rm -f *.o *~ a.out $(EXEC) $(TEST_EXEC)

pre_test:
	-rm $(TEST_OBJ) $(TEST_EXEC)
	./$(EXEC) -m all_messages.csv

run_test:
	./$(TEST_EXEC)
