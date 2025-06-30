import itertools
import math
import numpy as np
from collections import defaultdict

class Allocation:
    def __init__(self, bundles, valuations):
        self.bundles = bundles
        self.valuations = valuations
        self.n = len(bundles)

    def value(self, agent):
        return sum(self.valuations[agent][item] for item in self.bundles[agent])
    
    def value_of(self, agent, items):
        return sum(self.valuations[agent][item] for item in items)

    def is_efx(self):
        for i in range(self.n):
            for j in range(self.n):
                if i == j:
                    continue
                vi = self.value(i)
                for g in self.bundles[j]:
                    reduced = [x for x in self.bundles[j] if x != g]
                    vij = sum(self.valuations[i][x] for x in reduced)
                    if vi < vij:
                        return False
        return True

    def is_maximal(self): # doesn't work.
        """Returns the minimum utility across all agents (used for max-min fairness)."""
        return min(self.value(i) for i in range(self.n))

    def nash_welfare(self):
        product = 1
        for i in range(self.n):
            val = self.value(i)
            if val == 0:
                return 0
            product *= val
        return product

    def dominates(self, other):
        strictly_better = False
        for i in range(self.n):
            u1 = self.value(i)
            u2 = sum(self.valuations[i][item] for item in other.bundles[i])
            if u1 < u2:
                return False
            if u1 > u2:
                strictly_better = True
        return strictly_better
    
    def weak_envy(self, i, j):
        """Returns True if agent i weakly envies agent j."""
        # First check if there's any envy at all
        if not self.value_of(i, self.bundles[j]) > self.value(i):
            return False
        
        # If there's envy, check if it's strong envy
        # If it's strong envy, then it's not weak envy
        if self.strong_envy(i, j):
            return False
        
        # If there's envy but it's not strong envy, then it's weak envy
        return True

    def strong_envy(self, i, j):
        """Returns True if agent i strongly envies agent j.
        Strong envy means agent i envies agent j even after removing 
        the least valuable item (from agent i's perspective) from agent j's bundle."""
        
        # First check if there's any envy at all
        if not self.value_of(i, self.bundles[j]) > self.value(i):
            return False
        
        # Find the least valuable item in agent j's bundle from agent i's perspective
        if not self.bundles[j]:  # If bundle is empty, no strong envy possible
                return False
            
        least_valuable_item = min(self.bundles[j], key=lambda item: self.valuations[i][item])
        
        # Remove the least valuable item and check if envy still exists
        reduced_bundle = [x for x in self.bundles[j] if x != least_valuable_item]
        reduced_value = self.value_of(i, reduced_bundle)
        
        # If agent i still envies after removing the least valuable item, it's strong envy
        return reduced_value > self.value(i)
    
    def envy(self, i, j):
        """Returns True if agent i envies agent j."""
        return self.value_of(i, self.bundles[j]) > self.value(i)

    def value_of(self, agent, items):
        return sum(self.valuations[agent][item] for item in items)
    
    def most_envious_agent(self):
        max_gap = float('-inf')
        most_envious = None
        for i in range(self.n):
            for j in range(self.n):
                if i == j:
                    continue
                envy_gap = self.value_of(i, self.bundles[j]) - self.value(i)
                if envy_gap > max_gap:
                    max_gap = envy_gap
                    most_envious = i
        return most_envious, max_gap

def generate_valuations(n_agents, n_items, seed=None):
    import numpy as np
    if seed is not None:
        np.random.seed(seed)
    return np.random.randint(1, 11, size=(n_agents, n_items)).tolist()

# Distance functions between allocations

def swap_distance(allocation1, allocation2):
    """
    Calculate the minimum number of swaps needed to transform allocation1 into allocation2.
    Uses cycle decomposition of the swap graph.
    """
    n = len(allocation1)
    
    # Check if allocations are identical
    if allocation1 == allocation2:
        return 0
    
    # Create a mapping from items to their target agent
    item_to_target = {}
    for agent in range(n):
        for item in allocation2[agent]:
            item_to_target[item] = agent
    
    # Build the swap graph: edge (i,j) means agent i has an item that should go to agent j
    swap_graph = [[] for _ in range(n)]
    for agent in range(n):
        for item in allocation1[agent]:
            target_agent = item_to_target[item]
            if target_agent != agent:
                swap_graph[agent].append(target_agent)
    
    # Count cycles using DFS
    visited = [False] * n
    cycles = 0
    
    def dfs(node, start):
        if visited[node]:
            return node == start
        visited[node] = True
        for neighbor in swap_graph[node]:
            if dfs(neighbor, start):
                return True
        return False
    
    for i in range(n):
        if not visited[i] and swap_graph[i]:
            if dfs(i, i):
                cycles += 1
    
    # Minimum swaps = n - cycles
    return n - cycles

def normalized_euclidean_distance(allocation1, allocation2, valuations):
    """
    Calculate Euclidean distance between normalized utility vectors.
    Utilities are normalized by dividing by the sum of all valuations for each agent.
    """
    n = len(allocation1)
    
    # Calculate utility vectors for both allocations
    utils1 = []
    utils2 = []
    
    for agent in range(n):
        # Calculate total utility for agent in allocation1
        u1 = sum(valuations[agent][item] for item in allocation1[agent])
        utils1.append(u1)
        
        # Calculate total utility for agent in allocation2
        u2 = sum(valuations[agent][item] for item in allocation2[agent])
        utils2.append(u2)
    
    # Normalize utilities by dividing by sum of all valuations for each agent
    normalized_utils1 = []
    normalized_utils2 = []
    
    for agent in range(n):
        total_valuation = sum(valuations[agent])
        if total_valuation > 0:
            normalized_utils1.append(utils1[agent] / total_valuation)
            normalized_utils2.append(utils2[agent] / total_valuation)
        else:
            normalized_utils1.append(0)
            normalized_utils2.append(0)
    
    # Calculate Euclidean distance
    squared_diff_sum = sum((normalized_utils1[i] - normalized_utils2[i])**2 for i in range(n))
    return math.sqrt(squared_diff_sum)

def chebyshev_distance(allocation1, allocation2, valuations):
    """
    Calculate Chebyshev distance (L-infinity norm) between utility vectors.
    Returns the maximum absolute difference in utilities across all agents.
    """
    n = len(allocation1)
    
    max_diff = 0
    for agent in range(n):
        u1 = sum(valuations[agent][item] for item in allocation1[agent])
        u2 = sum(valuations[agent][item] for item in allocation2[agent])
        diff = abs(u1 - u2)
        max_diff = max(max_diff, diff)
    
    return max_diff

def earth_movers_distance(allocation1, allocation2):
    """
    Calculate Earth Mover's Distance between allocations.
    Treats items as units of mass and agents as locations.
    Cost of moving an item from agent i to agent j is 1 if i != j, 0 otherwise.
    """
    n = len(allocation1)
    
    # Count items per agent in each allocation
    count1 = [len(bundle) for bundle in allocation1]
    count2 = [len(bundle) for bundle in allocation2]
    
    # Calculate total flow needed
    total_flow = 0
    for i in range(n):
        # If agent i has more items in allocation1 than allocation2, 
        # we need to move the excess to other agents
        if count1[i] > count2[i]:
            excess = count1[i] - count2[i]
            total_flow += excess
    
    return total_flow

def envy_graph_distance(allocation1, allocation2, valuations):
    """
    Calculate the edit distance between envy graphs of two allocations.
    Returns the number of edge additions/deletions needed to transform one envy graph into another.
    """
    n = len(allocation1)
    
    def build_envy_graph(allocation):
        graph = set()
        for i in range(n):
            for j in range(n):
                if i != j:
                    # Calculate utility of agent i for their own bundle
                    u_own = sum(valuations[i][item] for item in allocation[i])
                    # Calculate utility of agent i for agent j's bundle
                    u_other = sum(valuations[i][item] for item in allocation[j])
                    if u_other > u_own:
                        graph.add((i, j))
        return graph
    
    envy_graph1 = build_envy_graph(allocation1)
    envy_graph2 = build_envy_graph(allocation2)
    
    # Calculate edit distance
    additions = len(envy_graph2 - envy_graph1)
    deletions = len(envy_graph1 - envy_graph2)
    
    return additions + deletions

def hamming_distance(allocation1, allocation2):
    """
    Calculate Hamming distance between allocations.
    Returns the number of items that need to be reassigned to transform one allocation into another.
    """
    n = len(allocation1)
    total_items = sum(len(bundle) for bundle in allocation1)
    
    # Count items that are in the same position in both allocations
    same_position = 0
    for agent in range(n):
        same_position += len(set(allocation1[agent]) & set(allocation2[agent]))
    
    # Hamming distance = total items - items in same position
    return total_items - same_position

def calculate_all_distances(allocation1, allocation2, valuations):
    """
    Calculate all distance metrics between two allocations.
    Returns a dictionary with all distance values.
    """
    return {
        'swap_distance': swap_distance(allocation1, allocation2),
        'normalized_euclidean': normalized_euclidean_distance(allocation1, allocation2, valuations),
        'chebyshev': chebyshev_distance(allocation1, allocation2, valuations),
        'earth_movers': earth_movers_distance(allocation1, allocation2),
        'envy_graph': envy_graph_distance(allocation1, allocation2, valuations),
        'hamming': hamming_distance(allocation1, allocation2)
    }