# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -I.

# Executable name
TARGET = mm_checker

# Source files
SRCS = mm.cpp allocation.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.cpp allocation.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Command to run the checker
run: all
	./$(TARGET)

