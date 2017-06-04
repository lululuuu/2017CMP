TARGET=CMP

CXX=g++ 
CXXFLAGS=-Wall -std=c++11 -O3

INCLUDES=$(wildcard *.h)
OBJS=$(shell ls *.cpp | sed "s/.cpp$$/.o/")

all: $(TARGET)

$(TARGET): $(OBJS) $(INCLUDES)
	$(CXX) $(CXXFLAGS) *.o -o $(TARGET)

%.o: %.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

clean:
	rm -rf $(TARGET) $(OBJS)

clean_all: clean
	rm -rf *.bin
	rm -rf *.rpt

testcase: $(TARGET)
	@rm -rf *.bin
	@rm -rf *.rpt
	ln -s ../testcase/*.bin .
	./$(TARGET)

open_testcase_%: $(TARGET)
	@echo "### open_testcase: $* ###"
	@rm -rf *.bin
	@rm -rf *.rpt
	ln -s ../open_testcase/$*/*.bin .
	./$(TARGET)
	diff ../open_testcase/$*/snapshot.rpt snapshot.rpt
	diff ../open_testcase/$*/report.rpt report.rpt
	@echo "##############################"

OPEN_TESTCASE=$(shell ls ../open_testcase | sed "s/^/open_testcase_/")

open_testcase: $(OPEN_TESTCASE)

clean_up:
	rm -f *.bin *.rpt
