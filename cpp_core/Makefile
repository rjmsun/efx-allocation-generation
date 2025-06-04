CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
TARGET = main
OBJS = main.o allocation.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp allocation.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

allocation.o: allocation.cpp allocation.hpp
	$(CXX) $(CXXFLAGS) -c allocation.cpp

clean:
	rm -f $(TARGET) $(OBJS)