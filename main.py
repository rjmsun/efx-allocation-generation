from efx import generate_valuations
from search_utils import generate_all_allocations
from efx import Allocation

def evaluate_and_print(allocs):
    max_nw = max(a.nash_welfare() for a in allocs)
    max_mm = max(a.is_maximal() for a in allocs)

    efx_list, mnw_list, mm_list, po_list = [], [], [], []

    for a in allocs:
        if a.is_efx():
            efx_list.append(a)
        if a.nash_welfare() == max_nw:
            mnw_list.append(a)
        if a.is_maximal() == max_mm:
            mm_list.append(a)
        if not any(other.dominates(a) for other in allocs):
            po_list.append(a)

    def print_examples(name, examples):
        print(f"\n=== {name} ({len(examples)} found) ===")
        for i, a in enumerate(examples[:5]):
            utilities = [a.value(j) for j in range(a.n)]
            print(f"Example {i+1}: Bundles={a.bundles} | Utilities={utilities}")

    print_examples("EFX allocations", efx_list)
    print_examples("MNW allocations", mnw_list)
    print_examples("Max-Min allocations", mm_list)
    print_examples("Pareto Optimal allocations", po_list)

if __name__ == "__main__":
    n_agents = 4
    n_items = 5
    valuations = generate_valuations(n_agents, n_items, seed=42)
    allocs = generate_all_allocations(n_agents, n_items, valuations)

    print("Valuations:")
    for i, row in enumerate(valuations):
        print(f"Agent {i+1}: {row}")
    
    evaluate_and_print(allocs)