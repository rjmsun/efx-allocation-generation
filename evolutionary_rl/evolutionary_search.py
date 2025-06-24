import numpy as np
import subprocess
import json
import os
import random
import time
import heapq
from datetime import datetime

# --- Configuration ---
# These can be adjusted for your search
NUM_AGENTS = 4
NUM_ITEMS = 9
CPP_EXECUTABLE = "./mm_checker"  # Path to your compiled C++ program

# --- Evolutionary Algorithm Parameters ---
POPULATION_SIZE = 100  # Number of utility matrices to test in each generation
NUM_GENERATIONS = 200  # How many cycles of evolution to run
ELITE_RATIO = 0.1      # Percentage of top performers to keep for the next generation
MUTATION_RATE = 0.2    # Probability of a single utility value changing
MUTATION_STRENGTH = 20 # How much a value can change when it mutates

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
            raise FileNotFoundError(f"C++ executable not found at {self.cpp_executable}. Please compile it first using 'make'.")

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
            # The C++ code now reads from stdin, so we pass the input string
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
        Calculates the 'fitness' of a utility matrix. A higher score means
        it is closer to being a counterexample.
        Fitness = 1 if it's a counterexample, otherwise 0.
        """
        results = self._run_cpp_oracle(utilities)
        if not results or "error" in results:
            return 0.0

        num_mm_allocations = results.get("mm_allocations_count", 0)
        num_mm_efx_allocations = results.get("mm_efx_allocations_count", 0)

        if num_mm_allocations > 0 and num_mm_efx_allocations == 0:
            # This is a counterexample!
            print("\n" + "="*20 + " COUNTEREXAMPLE FOUND! " + "="*20)
            print("UTILITY MATRIX:")
            for row in utilities:
                print(f"  {row}")
            print("\nThis instance has at least one MM allocation, none of which are EFX.")
            # Print MM allocations that are not EFX
            mm_allocs = results.get("mm_allocations", [])
            print("\nMM allocations (not EFX):")
            for alloc in mm_allocs:
                print(alloc)
                # Print strong envy and envy edges for this allocation
                try:
                    from efx import Allocation
                    from graphs import build_strong_envy_graph, build_envy_graph
                    allocation_obj = Allocation(alloc, utilities)
                    strong_envy_edges = build_strong_envy_graph(allocation_obj)
                    envy_edges = build_envy_graph(allocation_obj)
                    print("Strong envy edges:")
                    for i, j in strong_envy_edges:
                        print(f"{i} -> {j}")
                    print("Envy edges in general (weak or strong):")
                    for i, j in envy_edges:
                        print(f"{i} -> {j}")
                except Exception as e:
                    print(f"[Error printing envy edges: {e}]")
            print("="*62 + "\n")
            
            # Save counterexample to file
            self._save_counterexample_to_file(utilities, results)
            
            # Store the candidate for later analysis
            heapq.heappush(self.best_candidates, (1.0, utilities.tolist(), results))
            return 1.0
        
        return 0.0

    def _save_counterexample_to_file(self, utilities, results):
        """Save counterexample details to counterexample.txt"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
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
            f.write(f"{'='*80}\n\n")
        
        print(f"💾 Counterexample saved to counterexample.txt")

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
        
        # Add a small constant to handle all-zero fitness
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
        print(f"🧬 Starting evolutionary search for a counterexample...")
        print(f"   Config: {self.num_agents} agents, {self.num_items} items")
        print(f"   Population Size: {POPULATION_SIZE}, Generations: {NUM_GENERATIONS}")

        population = self._generate_initial_population()

        for gen in range(NUM_GENERATIONS):
            start_time = time.time()
            print(f"\n--- Generation {gen+1}/{NUM_GENERATIONS} ---")
            
            fitness_scores = [self.get_fitness_score(util) for util in population]
            
            best_fitness_in_gen = max(fitness_scores) if fitness_scores else 0
            if best_fitness_in_gen >= 1.0:
                print("🎉🎉🎉 Search successful! Counterexample found and printed above.")
                return 

            population = self._evolve(population, fitness_scores)
            
            gen_time = time.time() - start_time
            print(f"Generation {gen+1} complete. Best Fitness in Gen: {best_fitness_in_gen:.4f}. (Time: {gen_time:.2f}s)")

        print("\nSearch complete. No definitive counterexample found in the given generations.")


if __name__ == "__main__":
    # To use this script, you will need the 'nlohmann/json' library for C++.
    # If you are on a Debian-based system (like Ubuntu), you can install it with:
    # sudo apt-get install nlohmann-json3-dev
    # Then, you need to update your Makefile.
    
    searcher = CounterexampleSearch(NUM_AGENTS, NUM_ITEMS, CPP_EXECUTABLE)
    searcher.search()
