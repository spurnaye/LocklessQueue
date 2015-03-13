CXX=g++
CXXFLAGS= -std=c++11
LFLAGS= -lpthread
BINARY= queue_comparison

all:
	$(CXX) $(CXXFLAGS) $(LFLAGS) main_test.cpp LocklessSemaphore.cpp Semaphore.cpp -o $(BINARY)

clean:
	-@rm $(BINARY)
