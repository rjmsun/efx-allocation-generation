import itertools

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