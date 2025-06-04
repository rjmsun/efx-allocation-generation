from efx import Allocation
import itertools

def generate_all_allocations(n_agents, n_items, valuations):
    items = list(range(n_items))
    all_allocs = []
    for assignment in itertools.product(range(n_agents), repeat=n_items):
        bundles = [[] for _ in range(n_agents)]
        for item, agent in zip(items, assignment):
            bundles[agent].append(item)
        all_allocs.append(Allocation(bundles, valuations))
    return all_allocs

def evaluate(allocs):
    max_nw = max(a.nash_welfare() for a in allocs)
    stats = {"efx": 0, "mnw": 0, "mm": 0, "po": 0}
    for a in allocs:
        if a.is_efx(): stats["efx"] += 1
        if a.nash_welfare() == max_nw: stats["mnw"] += 1
        if a.is_maximal(): stats["mm"] += 1
        if not any(other.dominates(a) for other in allocs): stats["po"] += 1
    return stats