# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -I/opt/homebrew/Cellar/nlohmann-json/3.12.0/include

# Executables
TARGET = mm_checker
MM_CHECKER = mm_checker

# Source files
SOURCES = check_mm.cpp allocation.cpp
MM_SOURCES = mm.cpp allocation.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)
MM_OBJECTS = $(MM_SOURCES:.cpp=.o)

# Add efxpomo target
EFXPOMO = efxpomo
EFXPOMO_SOURCES = efxpomo.cpp allocation.cpp
EFXPOMO_OBJECTS = $(EFXPOMO_SOURCES:.cpp=.o)

# Default target
all: $(TARGET) $(MM_CHECKER) $(EFXPOMO)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

# Build mm_checker from mm.cpp
$(MM_CHECKER): $(MM_OBJECTS)
	$(CXX) $(MM_OBJECTS) -o $(MM_CHECKER)
	mv -f $(MM_CHECKER) ..

# Compile source files to object files
%.o: %.cpp allocation.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run mm_checker
run_mm: $(MM_CHECKER)
	./$(MM_CHECKER)

# Build efxpomo
$(EFXPOMO): $(EFXPOMO_OBJECTS)
	$(CXX) $(CXXFLAGS) $(EFXPOMO_OBJECTS) -o $(EFXPOMO)
	mv -f $(EFXPOMO) ..

efxpomo.o: efxpomo.cpp allocation.hpp
	$(CXX) $(CXXFLAGS) -c efxpomo.cpp -o efxpomo.o

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(MM_OBJECTS) $(TARGET) $(MM_CHECKER) $(EFXPOMO_OBJECTS) mm.txt
	rm -f ../mm_checker ../efxpomo

# Install dependencies (if needed)
install:
	# No external dependencies required for this project

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the executable (default)"
	@echo "  run     - Build and run the program"
	@echo "  run_mm  - Build and run mm_checker from mm.cpp"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"

.PHONY: all run run_mm clean install help

