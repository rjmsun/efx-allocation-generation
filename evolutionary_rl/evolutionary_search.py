import numpy as np
import subprocess
import json
import os
import random
import time
import heapq
from datetime import datetime
from pathlib import Path
import sys
sys.path.append(str(Path(__file__).resolve().parent.parent))
from efx import Allocation
from graphs import build_strong_envy_graph, build_envy_graph

# Configuration parameters
NUM_AGENTS = 3
NUM_ITEMS = 11
### make sure that mm_checker is in the same directory as this file 
### it should be if compiled using Makefile in cpp_core/
CPP_EXECUTABLE = "./mm_checker" 

# Evolutionary Algorithm Parameters
POPULATION_SIZE = 100  # number of utility matrices to test in each generation
NUM_GENERATIONS = 200  # number of evolution cycles to run
ELITE_RATIO = 0.1      # percentage of top performers to keep for the next generation
MUTATION_RATE = 0.2    # probability of a single utility value changing
MUTATION_STRENGTH = 20 # how much a value can change when it mutates

class CounterexampleSearch:
    """
    Uses an evolutionary algorithm to search the space of utility matrices
    for one that constitutes a counterexample to the MM -> EFX conjecture.
    """

    def __init__(self, num_agents, num_items, cpp_executable):
        self.num_agents = num_agents
        self.num_items = num_items
        self.cpp_executable = cpp_executable
        self.best_candidates = [] # Keep track of the top few candidates found
        if not os.path.exists(self.cpp_executable):
            raise FileNotFoundError(f"C++ executable not found at {self.cpp_executable}. Please compile it first in /cpp_core/ using 'make'.")

    def _run_cpp_oracle(self, utilities):
        """
        Runs the C++ checker with a given utility matrix.
        The C++ program must be modified to be non-interactive and output JSON.
        """
        # Prepare input for the C++ program
        proc_input = f"{self.num_agents}\n{self.num_items}\n"
        for row in utilities:
            proc_input += " ".join(map(str, row)) + "\n"

        try:
            process = subprocess.run(
                [self.cpp_executable, "--automate"], # Use the flag we added
                input=proc_input,
                capture_output=True,
                text=True,
                timeout=60  # 60-second timeout for safety
            )
            if process.returncode!= 0:
                return None
            return json.loads(process.stdout)
        except subprocess.TimeoutExpired:
            return None
        except json.JSONDecodeError:
            return None

    def get_fitness_score(self, utilities):
        """
        Fitness = 1 if it's a counterexample, otherwise 0.
        """
        results = self._run_cpp_oracle(utilities)
        if not results or "error" in results:
            return 0.0

        num_mm_allocations = results.get("mm_allocations_count", 0)
        num_mm_efx_allocations = results.get("mm_efx_allocations_count", 0)
        max_min_proportion = results.get("max_min_proportion", "N/A")

        # Compute min-optimal EFX allocations and proportion
        # We define a min-optimal EFX allocation as an 
        # EFX allocation where the minimum proportion of utility is maximized 
        # (assuming normalization of utilities)
        min_optimal_efx = []
        min_optimal_efx_proportion = 'N/A'
        try:
            proc_input = f"{self.num_agents}\n{self.num_items}\n"
            for row in utilities:
                proc_input += " ".join(map(str, row)) + "\n"
            result = subprocess.run([
                os.path.join(os.path.dirname(self.cpp_executable), "efxpomo"), "--automate"
            ], input=proc_input, capture_output=True, text=True, timeout=60)
            if result.returncode == 0:
                efx_json = json.loads(result.stdout)
                min_optimal_efx = efx_json.get("min_optimal_efx_allocations", [])
                if min_optimal_efx:
                    min_percentages = []
                    for alloc in min_optimal_efx:
                        min_prop = float('inf')
                        for agent_idx, bundle in enumerate(alloc):
                            bundle_value = sum(utilities[agent_idx][item] for item in bundle)
                            total_utility = sum(utilities[agent_idx])
                            prop = (bundle_value / total_utility) if total_utility > 0 else 0.0
                            min_prop = min(min_prop, prop)
                        min_percentages.append(min_prop)
                    if min_percentages:
                        min_optimal_efx_proportion = f"{max(min_percentages):.4f}"
        except Exception:
            pass

        if num_mm_allocations > 0 and num_mm_efx_allocations == 0:
            # counterexample found
            output_lines = []
            output_lines.append("="*80)
            output_lines.append("COUNTEREXAMPLE FOUND")
            output_lines.append("="*80)
            output_lines.append(f"Configuration: {self.num_agents} agents, {self.num_items} items")
            output_lines.append(f"MM allocations count: {num_mm_allocations}")
            output_lines.append(f"MM EFX allocations count: {num_mm_efx_allocations}")
            output_lines.append(f"Max-min proportion: {max_min_proportion}")
            output_lines.append(f"Min-optimal EFX proportion: {min_optimal_efx_proportion}")
            output_lines.append("")
            output_lines.append("Utility Matrix:")
            for row in utilities:
                output_lines.append("  " + str(row.tolist()))
            output_lines.append("")
            output_lines.append("MM Allocations (non EFX):")
            mm_allocs = results.get("mm_allocations", [])
            if mm_allocs:
                for alloc in mm_allocs:
                    try:
                        allocation_obj = Allocation(alloc, utilities)
                        for agent_idx, bundle in enumerate(allocation_obj.bundles):
                            bundle_value = allocation_obj.value_of(agent_idx, bundle)
                            total_utility = sum(utilities[agent_idx])
                            proportion = (bundle_value / total_utility) * 100 if total_utility > 0 else 0.0
                            output_lines.append(f"Agent {agent_idx} gets bundle {{{' '.join(map(str, bundle))} }}. (Value: {bundle_value}, {proportion:.2f}% of total utility)")
                        output_lines.append("")
                    except Exception as e:
                        output_lines.append(f"[Error printing allocation: {e}]")
            else:
                output_lines.append("  None")
            output_lines.append("")
            output_lines.append("Strong Envy Edges:")
            if mm_allocs:
                for alloc in mm_allocs:
                    try:
                        allocation_obj = Allocation(alloc, utilities)
                        strong_envy_edges = build_strong_envy_graph(allocation_obj)
                        if strong_envy_edges:
                            for i, j in strong_envy_edges:
                                output_lines.append(f"{i} -> {j}")
                        else:
                            output_lines.append("  None")
                        output_lines.append("")
                    except Exception as e:
                        output_lines.append(f"[Error printing strong envy edges: {e}]")
            else:
                output_lines.append("  None\n")
            output_lines.append("Envy Edges in General (weak or Strong)")
            if mm_allocs:
                for alloc in mm_allocs:
                    try:
                        allocation_obj = Allocation(alloc, utilities)
                        envy_edges = build_envy_graph(allocation_obj)
                        if envy_edges:
                            for i, j in envy_edges:
                                output_lines.append(f"{i} -> {j}")
                        else:
                            output_lines.append("  None")
                        output_lines.append("")
                    except Exception as e:
                        output_lines.append(f"[Error printing envy edges: {e}]")
            else:
                output_lines.append("  None\n")
            output_lines.append("Min-Optimal EFX allocations:")
            if min_optimal_efx:
                for idx, alloc in enumerate(min_optimal_efx, 1):
                    output_lines.append(f"Min-optimal EFX allocation #{idx}:")
                    for agent_idx, bundle in enumerate(alloc):
                        bundle_value = sum(utilities[agent_idx][item] for item in bundle)
                        total_utility = sum(utilities[agent_idx])
                        proportion = (bundle_value / total_utility) * 100 if total_utility > 0 else 0.0
                        output_lines.append(f"Agent {agent_idx} gets bundle {{{' '.join(map(str, bundle))} }}. (Value: {bundle_value}, {proportion:.2f}% of total utility)")
                    output_lines.append("")
            else:
                output_lines.append("  None\n")
            output_lines.append("="*80)
            output_lines.append("")
            # print + save all output
            output_str = "\n".join(output_lines)
            print(output_str)
            with open("counterexample.txt", "a") as f:
                f.write(output_str + "\n")
            # store candidate for later analysis
            heapq.heappush(self.best_candidates, (1.0, utilities.tolist(), results))
            return 1.0
        return 0.0

    def _save_counterexample_to_file(self, utilities, results):
        """Save counterexample details to counterexample.txt"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        from pathlib import Path
        import sys
        sys.path.append(str(Path(__file__).resolve().parent.parent))
        from efx import Allocation
        from graphs import build_strong_envy_graph, build_envy_graph
        with open("counterexample.txt", "a") as f:
            f.write(f"\n{'='*80}\n")
            f.write(f"COUNTEREXAMPLE FOUND - {timestamp}\n")
            f.write(f"{'='*80}\n")
            f.write(f"Configuration: {self.num_agents} agents, {self.num_items} items\n")
            f.write(f"MM allocations count: {results.get('mm_allocations_count', 0)}\n")
            f.write(f"MM EFX allocations count: {results.get('mm_efx_allocations_count', 0)}\n")
            f.write(f"Max-min proportion: {results.get('max_min_proportion', 'N/A')}\n\n")
            f.write("UTILITY MATRIX:\n")
            for i, row in enumerate(utilities):
                f.write(f"Agent {i}: {row}\n")
            f.write(f"\nDESCRIPTION: This instance has at least one MM allocation, none of which are EFX.\n")
            f.write(f"This constitutes a counterexample to the MM -> EFX conjecture.\n")
            mm_allocs = results.get("mm_allocations", [])
            f.write(f"\nMM allocations (not EFX):\n")
            for alloc in mm_allocs:
                try:
                    allocation_obj = Allocation(alloc, utilities)
                    for agent_idx, bundle in enumerate(allocation_obj.bundles):
                        bundle_value = allocation_obj.value_of(agent_idx, bundle)
                        total_utility = sum(utilities[agent_idx])
                        proportion = (bundle_value / total_utility) * 100 if total_utility > 0 else 0.0
                        f.write(f"Agent {agent_idx} gets bundle {{{' '.join(map(str, bundle))} }}. (Value: {bundle_value}, {proportion:.2f}% of total utility)\n")
                    f.write("\n")
                    strong_envy_edges = build_strong_envy_graph(allocation_obj)
                    f.write("Strong envy edges:\n")
                    for i, j in strong_envy_edges:
                        f.write(f"{i} -> {j}\n")
                    f.write("\n")
                    envy_edges = build_envy_graph(allocation_obj)
                    f.write("Envy edges in general (weak or strong):\n")
                    for i, j in envy_edges:
                        f.write(f"{i} -> {j}\n")
                    f.write("\n")
                except Exception as e:
                    f.write(f"[Error printing allocation/envy edges: {e}]\n")
            f.write(f"\n\n\n\n\n\n\n\n")
        print(f"Counterexample saved to counterexample.txt")

    def _generate_initial_population(self):
        """Creates the first generation of random utility matrices."""
        population = []
        for _ in range(POPULATION_SIZE):
            if random.random() < 0.5:
                matrix = np.random.randint(1, 101, size=(self.num_agents, self.num_items))
            else:
                matrix = self._generate_patterned_utilities()
            population.append(matrix)
        return population

    def _generate_patterned_utilities(self):
        """Generates utilities with specific structures to encourage conflict."""
        utils = np.random.randint(1, 20, size=(self.num_agents, self.num_items))
        for _ in range(self.num_items // 3):
            item_idx = random.randrange(self.num_items)
            p1, p2 = random.sample(range(self.num_agents), 2)
            utils[p1, item_idx] = random.randint(80, 100)
            utils[p2, item_idx] = random.randint(80, 100)
        return utils

    def _evolve(self, population, fitness_scores):
        """Creates the next generation from the current one using selection, crossover, and mutation."""
        elite_count = int(POPULATION_SIZE * ELITE_RATIO)
        
        # add a small constant to handle all-zero fitness
        probabilities = np.array(fitness_scores) + 0.01
        if probabilities.sum() == 0: # Should not happen with the +0.01
             probabilities = np.ones(len(population)) / len(population)
        else:
            probabilities /= probabilities.sum()
        
        elite_indices = np.argsort(fitness_scores)[-elite_count:]
        elites = [population[i] for i in elite_indices]
        new_population = elites[:]
        while len(new_population) < POPULATION_SIZE:
            parent1_idx, parent2_idx = np.random.choice(len(population), size=2, p=probabilities)
            parent1, parent2 = population[parent1_idx], population[parent2_idx]
            
            child = parent1.copy()
            crossover_point = random.randint(1, self.num_items - 1)
            child[:, crossover_point:] = parent2[:, crossover_point:]

            if random.random() < MUTATION_RATE:
                row_mut = random.randrange(self.num_agents)
                col_mut = random.randrange(self.num_items)
                mutation = random.randint(-MUTATION_STRENGTH, MUTATION_STRENGTH)
                child[row_mut, col_mut] = max(1, child[row_mut, col_mut] + mutation)
            
            new_population.append(child)
            
        return new_population

    def search(self):
        """Runs the main evolutionary search loop."""
        print(f"   Starting evolutionary search for a counterexample...")
        print(f"   Config: {self.num_agents} agents, {self.num_items} items")
        print(f"   Population Size: {POPULATION_SIZE}, Generations: {NUM_GENERATIONS}")

        population = self._generate_initial_population()

        for gen in range(NUM_GENERATIONS):
            start_time = time.time()
            print(f"\n--- Generation {gen+1}/{NUM_GENERATIONS} ---")
            
            fitness_scores = [self.get_fitness_score(util) for util in population]
            
            best_fitness_in_gen = max(fitness_scores) if fitness_scores else 0
            if best_fitness_in_gen >= 1.0:
                print("Search successful! Counterexample found and printed above.")
                return 

            population = self._evolve(population, fitness_scores)
            
            gen_time = time.time() - start_time
            print(f"Generation {gen+1} complete. Best Fitness in Gen: {best_fitness_in_gen:.4f}. (Time: {gen_time:.2f}s)")

        print("\nSearch complete. No definitive counterexample found in the given generations.")


if __name__ == "__main__":
    # To use this script, you will need the 'nlohmann/json' library for C++.
    # If you are on a Debian-based system (like Ubuntu), you can install it with:
    # sudo apt-get install nlohmann-json3-dev
    # on macOS, you can install it with:
    # brew install nlohmann-json
    # on Windows, you can install it with:
    # pip install nlohmann-json
    # Then, you need to update your Makefile in /cpp_core/ to include the new library.
    
    searcher = CounterexampleSearch(NUM_AGENTS, NUM_ITEMS, CPP_EXECUTABLE)
    searcher.search()
