# Makefile for matrix analysis
CXX = g++
CXXFLAGS = -std=c++11
TARGET = score
SOURCE = score.cpp

# Generated files to clean
GENERATED_FILES = score counterexample_analysis.txt uniform_analysis.txt poison_analysis.txt high_value_analysis.txt competitive_analysis.txt specialized_analysis.txt

.PHONY: all clean run

# Default target: clean, compile, and run
all: clean compile run

# Clean all generated files
clean:
	@echo "Cleaning generated files..."
	@rm -f $(GENERATED_FILES)

# Compile the program
compile: $(SOURCE)
	@echo "Compiling $(SOURCE)..."
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

# Run the program
run: compile
	@echo "Running $(TARGET)..."
	@./$(TARGET)

# Just compile without cleaning
build: compile

# Just run without recompiling (assumes program exists)
execute:
	@echo "Running $(TARGET)..."
	@./$(TARGET) 