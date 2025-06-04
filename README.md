# EFX Project: Testing Fair Division for Indivisible Goods

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
pip install numpy
```

---

## 📌 Next Steps

- Add support for champions / domination graphs
- Optimize search with pruning or heuristics
- Optional: re-implement in C++ for faster brute-force testing

---

## 📚 Reference

Chaudhury, Garg, Mehlhorn, Mehta (2020).  
*EFX Exists for Three Agents*.  
[arXiv:2004.07920](https://arxiv.org/abs/2004.07920)
