import networkx as nx
import matplotlib.pyplot as plt

def build_champion_graph(allocation):
    edges = []
    for i in range(allocation.n):
        for j in range(allocation.n):
            if i != j and allocation.value_of(i, allocation.bundles[j]) > allocation.value(i):
                edges.append((i, j))
    return edges

def build_strong_envy_graph(allocation):
    edges = []
    for i in range(allocation.n):
        for j in range(allocation.n):
            if i != j and allocation.strong_envy(i, j):
                edges.append((i, j))
    return edges

def build_weak_envy_graph(allocation):
    edges = []
    for i in range(allocation.n):
        for j in range(allocation.n):
            if i != j and allocation.weak_envy(i, j):
                edges.append((i, j))
    return edges

def build_envy_graph(allocation):
    edges = []
    for i in range(allocation.n):
        for j in range(allocation.n):
            if i != j and allocation.envy(i, j):
                edges.append((i, j))
    return edges


# Placeholder function for generating valuation distributions for testing
import numpy as np

def generate_val_distribution(num_agents, num_items, distribution="uniform", seed=None):
    if seed is not None:
        np.random.seed(seed)
    if distribution == "uniform":
        return np.random.randint(1, 11, size=(num_agents, num_items)).tolist()
    elif distribution == "binary":
        return np.random.choice([0, 10], size=(num_agents, num_items)).tolist()
    elif distribution == "sparse":
        return (np.random.binomial(1, 0.3, size=(num_agents, num_items)) * np.random.randint(1, 11, size=(num_agents, num_items))).tolist()
    elif distribution == "correlated":
        base = np.random.randint(1, 11, size=num_items)
        return (base + np.random.randint(0, 3, size=(num_agents, num_items))).tolist()
    else:
        raise ValueError("Unsupported distribution type.")


# Function to visualize a directed graph from edge list
def visualize_graph(edges, title="Graph", labels=None):
    G = nx.DiGraph()
    G.add_edges_from(edges)
    pos = nx.spring_layout(G, seed=42)

    plt.figure(figsize=(6, 4))
    nx.draw(G, pos, with_labels=True, node_color='skyblue', edge_color='gray', node_size=1000, font_size=12, arrows=True)

    if labels:
        nx.draw_networkx_labels(G, pos, labels=labels, font_size=12)
    
    plt.title(title)
    plt.tight_layout()
    plt.show()