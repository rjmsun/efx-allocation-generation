# EFX Allocation Generation: Fair Division Research Project

This project implements a comprehensive computational framework to analyze fairness and efficiency properties of indivisible allocations, with a primary focus on finding counterexamples to the **EFX existence conjecture** for four or more agents.

## Project Overview

### **Primary Research Goal**
Search for a **counterexample to the EFX existence conjecture** for \( n \geq 4 \) agents with additive valuations — i.e., a utility matrix with no EFX allocation.

### **Key Findings**
- **EFX and MM are logically independent**: An allocation can be maximin-optimal while creating justified envy, and vice versa
- **Counterexamples found**: The project has successfully identified multiple counterexamples for 3 agents, demonstrating that MM allocations need not be EFX
- **Evolutionary search effective**: The evolutionary algorithm has proven successful in finding difficult instances

---

## Project Structure

```
efx-allocation-generation/
├── cpp_core/                    # High-performance C++ implementation
│   ├── check_mm.cpp            # Brute force MM allocation finder and EFX tester
│   ├── mm.cpp                  # Alternative MM allocation brute force algorithm
│   ├── mms.cpp                 # Algorithm to find approximately MMS allocations
│   ├── efxpo.cpp               # Find Pareto optimal EFX allocations
│   ├── allocation.cpp          # EFX verification algorithms
│   ├── allocation.hpp          # Header definitions
│   ├── Makefile               # Build configuration
│   ├── run_mm_checker.sh      # Convenient run script
│   └── README.md              # C++ specific documentation
├── evolutionary_rl/            # Advanced search algorithms
│   ├── evolutionary_search.py  # Core evolutionary search for MM/EFX counterexamples
│   ├── rl_search.py           # Reinforcement learning enhanced search
│   ├── evolution_runner.py    # Automated search runner
│   └── graph_outputs/         # Visualization outputs
├── notebooks/                  # Jupyter notebooks for analysis
│   ├── analyze_graphs.ipynb    # Graph visualization and analysis
│   ├── mms_testing.ipynb      # MMS property analysis
│   └── test_imports.py
├── tests/                     # Unit tests
├── data/                      # Valuation data and results
├── efx.py                     # Core Python EFX implementation with allocation class
├── main.py                    # Simple test script
├── graphs.py                  # Graph visualization utilities and graph qualities
├── search_utils.py            # Search utility functions
├── counterexample.txt         # Found counterexamples
└── Makefile                   # Project-wide build configuration
```

---

## Quick Start Guide

### **1. Environment Setup**

```bash
# Clone the repository
git clone <repository-url>
cd efx-allocation-generation

# Install Python dependencies
pip install numpy matplotlib jupyter networkx

# Ensure you have a C++ compiler (g++ recommended)
g++ --version
```

### **2. Run the Core Evolutionary Search (Most Important)**

The evolutionary search is the core of this project. Here's how to run it:

```bash
# First, compile the C++ backend
cd cpp_core
make clean && make

# Return to project root
cd ..

# Run the evolutionary search
make
```

This will execute `evolutionary_search.py`, which is a simple evolutionary search that looks for MM/EFX counterexamples.

### **3. Run the C++ MM/EFX Checker**

```bash
cd cpp_core
make run  # Compiles and runs interactively
```

**Or use the convenience script:**
```bash
cd cpp_core
./run_mm_checker.sh
```

### **4. Run Python Analysis**

```bash
# From the project root
python main.py  # Simple test script
```

---

## Core Components

### **A. C++ Backend (`cpp_core/`)**

**Purpose**: High-performance brute-force verification for small instances

**Key Files**:
- **`check_mm.cpp`**: Brute force MM allocation finder and EFX tester
- **`mm.cpp`**: Alternative MM allocation brute force algorithm  
- **`mms.cpp`**: Algorithm to find approximately MMS allocations
- **`efxpo.cpp`**: Find Pareto optimal EFX allocations

**Key Features**:
- Fast EFX checking for up to 4 agents, 12 items
- MM (Maximin) allocation finding
- Interactive and automated modes
- JSON output for integration with Python

**Usage**:
```bash
cd cpp_core
make run
# Follow prompts for agents, items, and utility generation method
```

**Output**: Finds all MM allocations and checks which ones are EFX

### **B. Evolutionary Search (`evolutionary_rl/evolutionary_search.py`)**

**Purpose**: **Core component** - simple evolutionary search that looks for MM/EFX counterexamples

**Key Features**:
- Evolutionary algorithm to find difficult utility matrices
- Searches for instances where MM allocations are not EFX
- Automated verification of counterexamples
- Saves results to `counterexample.txt`

**Usage**:
```bash
# From project root after compiling C++ backend
make  # This runs evolutionary_search.py
```

**Configuration** (in `evolutionary_search.py`):
```python
NUM_AGENTS = 3          # Number of agents to test
NUM_ITEMS = 2           # Number of items to test
POPULATION_SIZE = 100   # Evolutionary population size
NUM_GENERATIONS = 200   # Number of generations
```

### **C. Python Analysis Components**

#### **`efx.py`** - Core EFX Implementation
**Purpose**: Contains the allocation class with EFX verification qualities

**Key Features**:
- EFX property verification
- Multiple fairness criteria (EFX, MNW, MM, PO)
- Allocation generation and evaluation
- Core allocation class implementation

#### **`graphs.py`** - Graph Visualization
**Purpose**: Contains graph qualities and visualization utilities

**Key Features**:
- Graph-based representation of allocations
- Visualization of fairness properties
- Network analysis of allocation structures
- Graph utilities for analysis

#### **`main.py`** - Simple Test Script
**Purpose**: Basic test script to verify functionality

**Usage**:
```bash
python main.py  # Run basic analysis
```

### **D. Jupyter Notebooks (`notebooks/`)**

**Purpose**: Interactive analysis and visualization

**Key Notebooks**:
- **`analyze_graphs.ipynb`**: Graph visualization and analysis - visualize the graphs and analyze allocation properties
- `mms_testing.ipynb`: MMS (Maximin Share) analysis

**Usage**:
```bash
jupyter notebook notebooks/
```

---

## Key Results and Counterexamples

### **Found Counterexamples**

The project has successfully identified multiple counterexamples demonstrating that **MM allocations need not be EFX**. Examples from `counterexample.txt`:

```
Configuration: 3 agents, 5 items
MM allocations count: 1
MM EFX allocations count: 0

UTILITY MATRIX:
Agent 0: [ 6 10  8 99  9]
Agent 1: [ 1 12 15 88  3]
Agent 2: [15  6 12 18 18]
```

### **Research Implications**

1. **MM ≠ EFX**: Maximin allocations can violate EFX
2. **Structural Patterns**: Counterexamples often involve high-value items that create envy
3. **Scalability**: The problem becomes computationally intensive for 4+ agents

---

## Advanced Usage

### **Custom Search Configuration**

Edit `evolutionary_rl/evolutionary_search.py`:
```python
# Search parameters
NUM_AGENTS = 4          # Test 4 agents
NUM_ITEMS = 6           # Test 6 items
POPULATION_SIZE = 200   # Larger population
NUM_GENERATIONS = 500   # More generations
MUTATION_RATE = 0.3     # Higher mutation rate
```

### **Batch Processing**

```bash
# Run multiple configurations
for agents in 3 4 5; do
    for items in 4 5 6; do
        echo "Testing $agents agents, $items items"
        # Modify evolutionary_search.py parameters
        python evolutionary_rl/evolutionary_search.py
    done
done
```

### **Integration with C++ Backend**

The evolutionary search can use the C++ backend for verification:
```python
# In evolutionary_search.py
CPP_EXECUTABLE = "./mm_checker"  # Path to compiled C++ program
```

---

## Performance Considerations

### **Computational Complexity**

- **Brute Force**: \(O(n^m)\) where \(n\) = agents, \(m\) = items
- **4 agents, 10 items**: ~1M allocations (feasible)
- **4 agents, 15 items**: ~1B allocations (very slow)
- **5+ agents**: Exponential growth

### **Optimization Strategies**

1. **Early Termination**: Stop when counterexample found
2. **Heuristic Search**: Use evolutionary algorithms for large instances
3. **Parallel Processing**: Multiple search threads
4. **Caching**: Store verified results

---

## Research Directions

### **Current Focus**
- Finding counterexamples for 4+ agents
- Characterizing "difficult" utility matrices
- Developing efficient search algorithms

### **Future Work**
- Extending to more agents
- Analyzing structural properties
- Developing approximation algorithms
- Exploring other fairness criteria

---

## Theoretical Background

### **Key Definitions**

- **EFX (Envy-Free up to any Item)**: For all agents \(i, j\) and any item \(g\) in \(j\)'s bundle, \(u_i(A_i) \geq u_i(A_j \setminus \{g\})\)

- **MM (Maximinality)**: Maximizes the minimum utility among all agents (after normalization of utilities)

- **MNW (Maximum Nash Welfare)**: Maximizes \(\prod u_i(A_i)\)

- **Pareto Optimality**: No reallocation makes all agents at least as well off

### **Research Context**
---

## Acknowledgments

- Based on theoretical work by Chaudhury et al.
- Inspired by computational fair division research