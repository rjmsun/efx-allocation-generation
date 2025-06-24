# Makefile for EFX Project
MAIN=evolutionary_rl/evolutionary_search.py
.PHONY: all run test clean cpp-build cpp-run
run:
	@echo "Running $(MAIN)..."
	python3 $(MAIN)
	
test:
	@echo "Running tests..."
	pytest tests/

clean:
	find . -type f -name "*.pyc" -delete
	find . -type d -name "__pycache__" -exec rm -r {} +

cpp-build:
	$(MAKE) -C cpp_core/

cpp-run: cpp-build
	./cpp_core/main

all: clean run