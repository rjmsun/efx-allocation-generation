# 🧠 EFX Allocation Generation: Fair Division Research Project

This project implements a complete, minimal framework to test fairness properties of **indivisible allocations** based on the 2020 paper *"EFX Exists for Three Agents"* by Chaudhury et al. The focus is on the rigorous analysis of allocations, without relying on machine learning.

---

## Goal

To systematically analyze allocations of items to agents and check for core fairness/efficiency properties such as:

- **EFX (Envy-Freeness up to any good)**  
- **Pareto Optimality**
- **Maximum Nash Welfare (MNW)**
- **Maximality (All items allocated)**
- **Domination between allocations**

We aim to extend and test the EFX conjecture for **4 agents**, where existence is still an open problem.

## Core Concepts Implemented

### EFX (Envy-Free up to any item)
For each pair of agents \( i 
eq j \), and for every good \( g \in A_j \),
\[
u_i(A_i) \geq u_i(A_j \setminus \{g\})
\]

### Pareto Optimality
No other allocation makes all agents at least as happy and one strictly happier.

### Maximality
All items are allocated.

### Nash Welfare
Product of agent utilities is maximized.

### Domination
An allocation A *dominates* B if no agent is worse off, and at least one is better off.

---

## 🧪 How to Run

Run the basic 4-agent, 5-item test:

```bash
python main.py
```

This will print:
- Valuations for each agent
- Count of allocations satisfying each fairness/efficiency property

---

## Dependencies

- Python 3.8+
- NumPy (for valuation generation)
- To be updated (pandas)

Install with:

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

## 📈 Performance Considerations

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

## 🔍 Research Directions

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

## 🐛 Troubleshooting

### **Common Issues**

1. **C++ Compilation Errors**:
   ```bash
   cd cpp_core
   make clean && make
   ```

2. **Python Import Errors**:
   ```bash
   pip install -r requirements.txt  # If requirements.txt exists
   # Or install manually:
   pip install numpy matplotlib jupyter networkx
   ```

3. **Memory Issues with Large Instances**:
   - Reduce `POPULATION_SIZE` in evolutionary search
   - Use smaller test configurations
   - Monitor system resources

### **Getting Help**

- Check the `counterexample.txt` file for successful runs
- Review the Jupyter notebooks for examples
- Examine the C++ output in `mm.txt`

---

## 📚 Reference

Chaudhury, Garg, Mehlhorn, Mehta (2020).  
*EFX Exists for Three Agents*.  
[arXiv:2004.07920](https://arxiv.org/abs/2004.07920)
