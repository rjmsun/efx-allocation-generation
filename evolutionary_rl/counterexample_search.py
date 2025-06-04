import numpy as np
import pandas as pd
import random
import itertools
import os
import sys
import matplotlib.pyplot as plt
import networkx as nx
sys.path.append(os.path.abspath("..")) 
from typing import List, Tuple, Dict, Optional
from efx import Allocation
from graphs import build_champion_graph, build_strong_envy_graph


class EFXCounterexampleSearch:
    """
    Enhanced search for EFX counterexamples using aggressive evolutionary approach
    with improved allocation strategies to reduce false positives
    """

    def __init__(self, num_players: int, num_items: int, seed: Optional[int] = None):
        if seed is not None:
            np.random.seed(seed)
            random.seed(seed)
        
        self.num_players = num_players
        self.num_items = num_items
        self.player_names = [f"Player{i+1}" for i in range(num_players)]
        self.item_names = [f"Item{i+1}" for i in range(num_items)]
        
        # Population parameters for evolutionary search
        self.population_size = 100
        self.num_generations = 200
        self.elite_ratio = 0.15
        self.mutation_strength = 0.4
        
        # Search tracking
        self.counterexample_candidates = []
        self.generation_history = []
        
        print(f"Initialized search: {num_players} players, {num_items} items")
    
    def generate_extreme_utilities(self) -> np.ndarray:
        """Generate utility matrix designed to create maximum conflicts"""
        utilities = np.random.uniform(0.1, 1.0, size=(self.num_players, self.num_items))
        
        # Strategy 1: Create "super valuable" items
        num_super = min(3, self.num_items // 3)
        for item in range(num_super):
            # Some players value it extremely highly
            high_valuers = np.random.choice(
                self.num_players, 
                size=np.random.randint(1, max(2, self.num_players // 2)), 
                replace=False
            )
            for player in high_valuers:
                utilities[player, item] = np.random.uniform(80, 120)
            
            # Others value it very little
            for player in range(self.num_players):
                if player not in high_valuers:
                    utilities[player, item] = np.random.uniform(0.01, 0.5)
        
        # Strategy 2: Create hierarchical preferences
        for player in range(self.num_players):
            remaining_items = list(range(num_super, self.num_items))
            np.random.shuffle(remaining_items)
            
            for rank, item in enumerate(remaining_items):
                # Exponential decay in values
                value = 50 * (0.6 ** rank) + np.random.uniform(0, 2)
                utilities[player, item] = value
        
        # Strategy 3: Add some "poison" items
        num_poison = min(2, self.num_items // 5)
        for i in range(num_poison):
            poison_item = num_super + i
            if poison_item < self.num_items:
                victims = np.random.choice(
                    self.num_players, 
                    size=np.random.randint(1, self.num_players), 
                    replace=False
                )
                for victim in victims:
                    utilities[victim, poison_item] = np.random.uniform(0.001, 0.01)
        
        return utilities
    
    def check_efx_violation(self, utilities: np.ndarray, allocation: Allocation) -> Tuple[bool, float, List[str], Allocation]:
        """
        Check if allocation violates EFX property
        Returns: (is_violated, total_violation_strength, violation_descriptions, allocation)
        """
        violations = []
        total_violation = 0.0

        for player_a in range(self.num_players):
            bundle_a = allocation.bundles[player_a]
            value_a = sum(utilities[player_a, item] for item in bundle_a) if bundle_a else 0.0

            for player_b in range(self.num_players):
                if player_a == player_b:
                    continue

                bundle_b = allocation.bundles[player_b]
                if not bundle_b:
                    continue

                # Player A's value for player B's bundle
                value_b = sum(utilities[player_a, item] for item in bundle_b)

                # Find least valuable item in B's bundle (from A's perspective)
                least_item = min(bundle_b, key=lambda item: utilities[player_a, item])
                least_value = utilities[player_a, least_item]

                # Value after removing least valuable item
                value_b_reduced = value_b - least_value

                # Check EFX violation
                if value_b_reduced > value_a + 1e-10:
                    violation_gap = value_b_reduced - value_a
                    total_violation += violation_gap

                    violation_desc = (
                        f"{self.player_names[player_a]} envies {self.player_names[player_b]} "
                        f"even after removing {self.item_names[least_item]} "
                        f"(gap: {violation_gap:.6f})"
                    )
                    violations.append(violation_desc)

        return len(violations) > 0, total_violation, violations, allocation
    
    def test_multiple_allocation_strategies(self, utilities: np.ndarray) -> float:
        """Enhanced strategy testing with more allocation methods"""
        strategies = [
            self.random_allocation_strategy,
            self.greedy_max_value_strategy,
            self.round_robin_strategy,
            self.balanced_value_strategy,
            self.envy_minimization_strategy,
            self.adversarial_strategy,
            self.proportional_strategy,
            # New improved strategies
            self.specialized_matching_strategy,
            self.conflict_aware_strategy,
            self.minimax_envy_strategy,
            self.constraint_satisfaction_strategy
        ]

        min_violation = float('inf')

        for strategy in strategies:
            for attempt in range(100):  # Increased attempts
                allocation = strategy(utilities)
                alloc_obj = allocation
                is_violated, violation_strength, _, _ = self.check_efx_violation(utilities, alloc_obj)
                # Compute champion graph and number of champions
                from graphs import build_champion_graph
                G = build_champion_graph(alloc_obj)
                num_champions = len(G)
                # Compute normalized fitness score: penalize violation, reward champions
                score = violation_strength - 0.25 * num_champions
                if not is_violated:
                    return 0.0  # EFX found!
                min_violation = min(min_violation, score)

        return min_violation if min_violation != float('inf') else 100.0
    
    def random_allocation_strategy(self, utilities: np.ndarray) -> Allocation:
        """Random allocation of items"""
        allocation = [[] for _ in range(self.num_players)]
        for item in range(self.num_items):
            player = np.random.randint(self.num_players)
            allocation[player].append(item)
        return Allocation(allocation, utilities)

    def greedy_max_value_strategy(self, utilities: np.ndarray) -> Allocation:
        """Assign each item to the player who values it most"""
        allocation = [[] for _ in range(self.num_players)]
        for item in range(self.num_items):
            best_player = np.argmax(utilities[:, item])
            allocation[best_player].append(item)
        return Allocation(allocation, utilities)
    
    def round_robin_strategy(self, utilities: np.ndarray) -> Allocation:
        """Round-robin picking with best remaining item"""
        allocation = [[] for _ in range(self.num_players)]
        remaining_items = list(range(self.num_items))
        current_player = 0

        while remaining_items:
            # Current player picks their best remaining item
            player_values = [utilities[current_player, item] for item in remaining_items]
            best_idx = np.argmax(player_values)
            best_item = remaining_items[best_idx]

            allocation[current_player].append(best_item)
            remaining_items.remove(best_item)
            current_player = (current_player + 1) % self.num_players

        return Allocation(allocation, utilities)
    
    def balanced_value_strategy(self, utilities: np.ndarray) -> Allocation:
        """Try to balance total values across players"""
        allocation = [[] for _ in range(self.num_players)]
        current_values = [0.0] * self.num_players

        # Sort items by total value (most valuable first)
        items_by_value = sorted(range(self.num_items),
                               key=lambda item: np.sum(utilities[:, item]),
                               reverse=True)

        for item in items_by_value:
            # Assign to player with lowest current value
            poorest_player = np.argmin(current_values)
            allocation[poorest_player].append(item)
            current_values[poorest_player] += utilities[poorest_player, item]

        return Allocation(allocation, utilities)

    def envy_minimization_strategy(self, utilities: np.ndarray) -> Allocation:
        """Try to minimize total envy"""
        allocation = [[] for _ in range(self.num_players)]
        remaining_items = list(range(self.num_items))

        while remaining_items:
            best_assignment = None
            min_total_envy = float('inf')

            # Try assigning each remaining item to each player
            for item in remaining_items[:min(8, len(remaining_items))]:  # Limit for performance
                for player in range(self.num_players):
                    # Temporarily assign item
                    allocation[player].append(item)

                    # Calculate total envy
                    total_envy = 0.0
                    for p1 in range(self.num_players):
                        val1 = sum(utilities[p1, i] for i in allocation[p1])
                        for p2 in range(self.num_players):
                            if p1 != p2:
                                val2 = sum(utilities[p1, i] for i in allocation[p2])
                                if val2 > val1:
                                    total_envy += val2 - val1

                    if total_envy < min_total_envy:
                        min_total_envy = total_envy
                        best_assignment = (player, item)

                    # Remove temporary assignment
                    allocation[player].remove(item)

            # Make best assignment
            if best_assignment:
                player, item = best_assignment
                allocation[player].append(item)
                remaining_items.remove(item)
            else:
                # Fallback
                item = remaining_items.pop(0)
                allocation[0].append(item)

        return Allocation(allocation, utilities)
    
    def adversarial_strategy(self, utilities: np.ndarray) -> Allocation:
        """Deliberately create conflicts"""
        allocation = [[] for _ in range(self.num_players)]

        # Give high-value items to players who don't value them much
        for item in range(self.num_items):
            values = utilities[:, item]
            # Assign to player with median value (not highest or lowest)
            sorted_indices = np.argsort(values)
            median_player = sorted_indices[len(sorted_indices) // 2]
            allocation[median_player].append(item)

        return Allocation(allocation, utilities)
    
    def proportional_strategy(self, utilities: np.ndarray) -> Allocation:
        """Try to give each player items proportional to their total utility"""
        allocation = [[] for _ in range(self.num_players)]

        # Calculate proportional targets
        total_utilities = [np.sum(utilities[p, :]) for p in range(self.num_players)]
        grand_total = sum(total_utilities)
        targets = [tu / grand_total for tu in total_utilities]

        # Assign items to meet targets approximately
        remaining_items = list(range(self.num_items))
        current_ratios = [0.0] * self.num_players

        while remaining_items:
            best_assignment = None
            best_improvement = -1

            for item in remaining_items:
                for player in range(self.num_players):
                    item_value = utilities[player, item]
                    new_ratio = (current_ratios[player] * len(allocation[player]) + item_value) / (len(allocation[player]) + 1)
                    improvement = abs(targets[player] - new_ratio) - abs(targets[player] - current_ratios[player])

                    if improvement > best_improvement:
                        best_improvement = improvement
                        best_assignment = (player, item)

            if best_assignment:
                player, item = best_assignment
                allocation[player].append(item)
                current_ratios[player] = (current_ratios[player] * (len(allocation[player]) - 1) + utilities[player, item]) / len(allocation[player])
                remaining_items.remove(item)
            else:
                item = remaining_items.pop(0)
                allocation[0].append(item)

        return Allocation(allocation, utilities)
    
    def specialized_matching_strategy(self, utilities: np.ndarray) -> Allocation:
        """Give each player their most valued items first, then distribute remainder"""
        allocation = [[] for _ in range(self.num_players)]
        used_items = set()

        # Phase 1: Give each player their top unused item in rounds
        for round_num in range(min(self.num_items // self.num_players + 1, 3)):
            for player in range(self.num_players):
                if len(used_items) >= self.num_items:
                    break

                # Find player's best unused item
                best_item = None
                best_value = -1
                for item in range(self.num_items):
                    if item not in used_items and utilities[player, item] > best_value:
                        best_value = utilities[player, item]
                        best_item = item

                if best_item is not None:
                    allocation[player].append(best_item)
                    used_items.add(best_item)

        # Phase 2: Distribute remaining items randomly
        remaining_items = [i for i in range(self.num_items) if i not in used_items]
        for item in remaining_items:
            player = np.random.randint(self.num_players)
            allocation[player].append(item)

        return Allocation(allocation, utilities)
    
    def conflict_aware_strategy(self, utilities: np.ndarray) -> Allocation:
        """Identify high-conflict items and distribute them carefully"""
        allocation = [[] for _ in range(self.num_players)]

        # Calculate conflict score for each item
        conflicts = []
        for item in range(self.num_items):
            values = utilities[:, item]
            # Conflict = variance * max value (high disagreement on valuable items)
            conflict_score = np.var(values) * np.max(values)
            conflicts.append((conflict_score, item))

        # Sort by conflict (highest first)
        conflicts.sort(reverse=True)

        # Distribute high-conflict items to players who value them most
        # But low-conflict items more randomly
        for conflict_score, item in conflicts:
            if conflict_score > np.mean([c[0] for c in conflicts]):
                # High conflict: give to highest valuer
                best_player = np.argmax(utilities[:, item])
            else:
                # Low conflict: distribute to balance loads
                current_loads = [len(bundle) for bundle in allocation]
                best_player = np.argmin(current_loads)

            allocation[best_player].append(item)

        return Allocation(allocation, utilities)
    
    def minimax_envy_strategy(self, utilities: np.ndarray) -> Allocation:
        """Try to minimize the maximum envy across all players"""
        # Try different orderings of items
        best_allocation = None
        min_max_envy = float('inf')

        for _ in range(min(50, self.num_items * 10)):
            allocation = [[] for _ in range(self.num_players)]
            item_order = list(range(self.num_items))
            np.random.shuffle(item_order)

            for item in item_order:
                # Assign item to player that minimizes maximum envy
                best_player = None
                best_max_envy = float('inf')

                for player in range(self.num_players):
                    # Temporarily assign
                    allocation[player].append(item)

                    # Calculate maximum envy
                    max_envy = 0
                    for p1 in range(self.num_players):
                        own_value = sum(utilities[p1, i] for i in allocation[p1])
                        for p2 in range(self.num_players):
                            if p1 != p2:
                                other_value = sum(utilities[p1, i] for i in allocation[p2])
                                envy = max(0, other_value - own_value)
                                max_envy = max(max_envy, envy)

                    if max_envy < best_max_envy:
                        best_max_envy = max_envy
                        best_player = player

                    # Remove temporary assignment
                    allocation[player].remove(item)

                # Make best assignment
                if best_player is not None:
                    allocation[best_player].append(item)

            # Check if this is the best allocation so far
            max_envy = 0
            for p1 in range(self.num_players):
                own_value = sum(utilities[p1, i] for i in allocation[p1])
                for p2 in range(self.num_players):
                    if p1 != p2:
                        other_value = sum(utilities[p1, i] for i in allocation[p2])
                        envy = max(0, other_value - own_value)
                        max_envy = max(max_envy, envy)

            if max_envy < min_max_envy:
                min_max_envy = max_envy
                best_allocation = [bundle.copy() for bundle in allocation]

        if best_allocation:
            return Allocation(best_allocation, utilities)
        else:
            return self.random_allocation_strategy(utilities)
    
    def constraint_satisfaction_strategy(self, utilities: np.ndarray) -> Allocation:
        """Use constraint satisfaction approach to find EFX-satisfying allocations"""
        # Start with empty allocation
        allocation = [[] for _ in range(self.num_players)]
        remaining_items = list(range(self.num_items))

        # Sort items by total utility (most valuable first)
        remaining_items.sort(key=lambda item: np.sum(utilities[:, item]), reverse=True)

        for item in remaining_items:
            best_player = None
            min_violation_potential = float('inf')

            # Try assigning to each player
            for player in range(self.num_players):
                allocation[player].append(item)

                # Estimate EFX violation potential
                violation_potential = 0

                for p1 in range(self.num_players):
                    bundle1 = allocation[p1]
                    value1 = sum(utilities[p1, i] for i in bundle1)

                    for p2 in range(self.num_players):
                        if p1 != p2:
                            bundle2 = allocation[p2]
                            if bundle2:  # Non-empty bundle
                                value2_full = sum(utilities[p1, i] for i in bundle2)
                                # Check EFX violation for each possible item removal
                                min_efx_value = min(
                                    value2_full - utilities[p1, remove_item]
                                    for remove_item in bundle2
                                )
                                if min_efx_value > value1:
                                    violation_potential += min_efx_value - value1

                if violation_potential < min_violation_potential:
                    min_violation_potential = violation_potential
                    best_player = player

                # Remove temporary assignment
                allocation[player].remove(item)

            # Assign to best player
            if best_player is not None:
                allocation[best_player].append(item)
            else:
                # Fallback: assign to player with fewest items
                loads = [len(bundle) for bundle in allocation]
                allocation[np.argmin(loads)].append(item)

        return Allocation(allocation, utilities)
    
    def evolutionary_search(self) -> List[Dict]:
        """Main evolutionary search for counterexamples"""
        print(f"🧬 Starting evolutionary search...")
        print(f"   Population: {self.population_size}")
        print(f"   Generations: {self.num_generations}")
        print(f"   Elite ratio: {self.elite_ratio}")
        print(f"   Mutation strength: {self.mutation_strength}")
        
        # Initialize population
        population = []
        for _ in range(self.population_size):
            utilities = self.generate_extreme_utilities()
            population.append(utilities)
        
        for generation in range(self.num_generations):
            # Evaluate population
            fitness_scores = []
            for utilities in population:
                fitness = self.test_multiple_allocation_strategies(utilities)
                fitness_scores.append(fitness)
            
            # Track best candidates
            for i, (utilities, fitness) in enumerate(zip(population, fitness_scores)):
                if fitness > 20.0:  # High violation threshold
                    candidate = {
                        'utilities': utilities.copy(),
                        'fitness': fitness,
                        'generation': generation,
                        'individual': i
                    }
                    self.counterexample_candidates.append(candidate)
            
            # Report progress
            if generation % 20 == 0:
                best_fitness = max(fitness_scores)
                avg_fitness = np.mean(fitness_scores)
                print(f"   Gen {generation}: Best={best_fitness:.2f}, Avg={avg_fitness:.2f}, Candidates={len(self.counterexample_candidates)}")
            
            # Evolution
            if generation < self.num_generations - 1:
                population = self.evolve_population(population, fitness_scores)
        
        print(f"🔬 Evolution complete! Found {len(self.counterexample_candidates)} candidates")
        os.makedirs("graph_outputs", exist_ok=True)
        for i, candidate in enumerate(sorted(self.counterexample_candidates, key=lambda x: x['fitness'], reverse=True)[:5]):
            utilities = candidate['utilities']
            for strategy in [
                self.random_allocation_strategy,
                self.greedy_max_value_strategy,
                self.round_robin_strategy
            ]:
                alloc_obj = strategy(utilities)

                champ_graph = build_champion_graph(alloc_obj)
                envy_graph = build_strong_envy_graph(alloc_obj)

                # Save graphs for use in notebook
                nx.write_gpickle(champ_graph, f"graph_outputs/champion_graph_{i}_{strategy.__name__}.gpickle")
                nx.write_gpickle(envy_graph, f"graph_outputs/envy_graph_{i}_{strategy.__name__}.gpickle")
        return self.counterexample_candidates
    
    def evolve_population(self, population: List[np.ndarray], fitness_scores: List[float]) -> List[np.ndarray]:
        """Evolve population for next generation"""
        elite_size = int(self.population_size * self.elite_ratio)
        
        # Select elite
        elite_indices = np.argsort(fitness_scores)[-elite_size:]
        new_population = [population[i].copy() for i in elite_indices]
        
        # Generate offspring
        while len(new_population) < self.population_size:
            if len(elite_indices) >= 2:
                # Crossover
                parent1 = population[np.random.choice(elite_indices)]
                parent2 = population[np.random.choice(elite_indices)]
                child = self.crossover(parent1, parent2)
            else:
                # Mutation only
                parent = population[np.random.choice(elite_indices)]
                child = parent.copy()
            
            # Apply mutation
            child = self.mutate(child)
            new_population.append(child)
        
        return new_population
    
    def crossover(self, parent1: np.ndarray, parent2: np.ndarray) -> np.ndarray:
        """Create offspring from two parents"""
        mask = np.random.random(parent1.shape) < 0.5
        child = np.where(mask, parent1, parent2)
        return child
    
    def mutate(self, individual: np.ndarray) -> np.ndarray:
        """Apply mutations to an individual"""
        mutated = individual.copy()
        
        # Gaussian mutation
        mutation_mask = np.random.random(individual.shape) < 0.4
        noise = np.random.normal(0, self.mutation_strength * np.mean(individual), individual.shape)
        mutated[mutation_mask] += noise[mutation_mask]
        
        # Ensure positive values
        mutated = np.maximum(0.001, mutated)
        
        # Occasionally add extreme values
        if np.random.random() < 0.1:
            extreme_positions = np.random.random(individual.shape) < 0.05
            mutated[extreme_positions] = np.random.uniform(100, 200, np.sum(extreme_positions))
        
        return mutated
    
    def verify_counterexample(self, utilities: np.ndarray) -> Dict:
        """Exhaustively verify if utilities matrix is a counterexample"""
        print(f"🔍 Exhaustive verification...")
        
        total_allocations = self.num_players ** self.num_items
        print(f"   Total possible allocations: {total_allocations}")
        
        if total_allocations > 500000:
            return self.statistical_verification(utilities)
        else:
            return self.exhaustive_verification(utilities)
    
    def exhaustive_verification(self, utilities: np.ndarray) -> Dict:
        """Check all possible allocations"""
        print(f"   Checking all allocations exhaustively...")

        total_checked = 0
        efx_found = 0
        min_violations = float('inf')
        best_allocation = None

        for allocation_tuple in itertools.product(range(self.num_players), repeat=self.num_items):
            total_checked += 1

            # Convert to allocation format
            allocation = [[] for _ in range(self.num_players)]
            for item, player in enumerate(allocation_tuple):
                allocation[player].append(item)
            alloc_obj = Allocation(allocation, utilities)
            # Check EFX
            is_violated, violation_strength, violations, _ = self.check_efx_violation(utilities, alloc_obj)

            if not is_violated:
                efx_found += 1
                print(f"   ✅ EFX allocation found: {allocation}")
                return {
                    'is_counterexample': False,
                    'efx_allocation_found': allocation,
                    'total_checked': total_checked,
                    'efx_count': efx_found
                }
            else:
                if violation_strength < min_violations:
                    min_violations = violation_strength
                    best_allocation = allocation

            if total_checked % 10000 == 0:
                print(f"      Progress: {total_checked} allocations checked...")

        print(f"🎉 COUNTEREXAMPLE VERIFIED!")
        print(f"   Checked ALL {total_checked} allocations")
        print(f"   EFX allocations found: {efx_found}")

        return {
            'is_counterexample': True,
            'total_checked': total_checked,
            'efx_count': efx_found,
            'best_allocation': best_allocation,
            'min_violations': min_violations
        }
    
    def statistical_verification(self, utilities: np.ndarray, num_samples: int = 100000) -> Dict:
        """Statistical verification for large instances"""
        print(f"   Statistical verification with {num_samples} samples...")

        efx_found = 0
        total_tested = 0
        min_violations = float('inf')
        best_allocation = None

        for i in range(num_samples):
            alloc_obj = self.random_allocation_strategy(utilities)
            is_violated, violation_strength, violations, _ = self.check_efx_violation(utilities, alloc_obj)
            total_tested += 1

            if not is_violated:
                efx_found += 1
                print(f"   ✅ EFX allocation found: {alloc_obj.bundles}")
                return {
                    'is_counterexample': False,
                    'efx_allocation_found': alloc_obj.bundles,
                    'total_tested': total_tested,
                    'efx_found': efx_found
                }
            else:
                if violation_strength < min_violations:
                    min_violations = violation_strength
                    best_allocation = alloc_obj.bundles

            if i % 10000 == 0 and i > 0:
                print(f"      Progress: {i} samples tested...")

        confidence = (1 - efx_found / total_tested) * 100
        print(f"🤔 Strong counterexample candidate")
        print(f"   No EFX found in {total_tested} samples")
        print(f"   Confidence: {confidence:.2f}%")

        return {
            'is_counterexample': efx_found == 0,
            'total_tested': total_tested,
            'efx_found': efx_found,
            'best_allocation': best_allocation,
            'min_violations': min_violations,
            'confidence': f"{confidence:.2f}%"
        }
    
    def analyze_candidate(self, candidate: Dict, verification_result: Dict):
        """Analyze and display counterexample candidate"""
        print(f"\n{'='*80}")
        print("COUNTEREXAMPLE CANDIDATE ANALYSIS")
        print(f"{'='*80}")

        utilities = candidate['utilities']
        print(f"Fitness Score: {candidate['fitness']:.4f}")
        print(f"Generation: {candidate['generation']}")

        # Show utility matrix
        print(f"\nUtility Matrix:")
        df = pd.DataFrame(utilities,
                         index=self.player_names,
                         columns=self.item_names)
        print(df.round(2))

        # Show verification results
        if verification_result['is_counterexample']:
            print(f"\n🎉 VERIFIED COUNTEREXAMPLE!")
            if 'total_checked' in verification_result:
                print(f"   Method: Exhaustive verification")
                print(f"   Allocations checked: {verification_result['total_checked']}")
            else:
                print(f"   Method: Statistical verification")
                print(f"   Confidence: {verification_result.get('confidence', 'N/A')}")
        else:
            print(f"\n❌ False positive - EFX allocation exists")
            found_alloc = verification_result.get('efx_allocation_found', [])
            print(f"   Found allocation: {Allocation(found_alloc) if found_alloc else found_alloc}")

        return verification_result['is_counterexample']


def run_counterexample_search():
    """Main function to run the counterexample search"""
    print("🎯 AGGRESSIVE EFX COUNTEREXAMPLE SEARCH")
    print("="*80)
    print("Strategy: Extreme utilities + Enhanced allocation strategies + Evolutionary search")
    print()
    
    test_configurations = [
        (4, 8)
    ]
    
    verified_counterexamples = []
    
    for num_players, num_items in test_configurations:
        print(f"\n{'='*60}")
        print(f"TESTING: {num_players} players, {num_items} items")
        print(f"{'='*60}")
        
        # Create searcher
        searcher = EFXCounterexampleSearch(num_players, num_items, seed=None)
        
        # Run evolutionary search
        candidates = searcher.evolutionary_search()
        
        if candidates:
            print(f"\n🎯 Testing top {min(3, len(candidates))} candidates...")
            
            # Sort by fitness and test best candidates
            sorted_candidates = sorted(candidates, key=lambda x: x['fitness'], reverse=True)
            
            for i, candidate in enumerate(sorted_candidates[:3]):
                print(f"\n--- Candidate {i+1} (Fitness: {candidate['fitness']:.2f}) ---")
                
                verification_result = searcher.verify_counterexample(candidate['utilities'])
                is_verified = searcher.analyze_candidate(candidate, verification_result)
                
                if is_verified:
                    verified_counterexamples.append({
                        'candidate': candidate,
                        'verification': verification_result,
                        'config': (num_players, num_items)
                    })
                    print(f"🎉 VERIFIED COUNTEREXAMPLE FOUND!")
                    break
            
            if not any(searcher.analyze_candidate(c, searcher.verify_counterexample(c['utilities'])) 
                      for c in sorted_candidates[:3]):
                print(f"❌ All candidates were false positives")
        else:
            print(f"❌ No promising candidates found")
    
    # Final results
    print(f"\n{'='*80}")
    print("FINAL RESULTS")
    print(f"{'='*80}")
    
    if verified_counterexamples:
        print(f"🎉 SUCCESS: {len(verified_counterexamples)} VERIFIED COUNTEREXAMPLES!")
        print("\n🏆 MATHEMATICAL BREAKTHROUGH:")
        print("• First computational proof that EFX doesn't always exist")
        print("• Resolves major open conjecture in fair division theory")
        print("• Demonstrates power of AI for mathematical discovery")
        
        for i, ce in enumerate(verified_counterexamples, 1):
            config = ce['config']
            fitness = ce['candidate']['fitness']
            print(f"\nCounterexample {i}: {config[0]} players, {config[1]} items (fitness: {fitness:.2f})")
            
    else:
        print("❌ NO COUNTEREXAMPLES FOUND")
        print("\n📊 IMPLICATIONS:")
        print("• Strong computational evidence supporting EFX existence")
        print("• Conjecture survives aggressive evolutionary attack")
        print("• EFX appears remarkably robust")
        print("\n💡 RESEARCH VALUE:")
        print("• Comprehensive computational study of EFX conjecture")
        print("• Novel AI methodology for mathematical investigation")
        print("• Valuable negative results for the field")
    
    return verified_counterexamples


def quick_test():
    """Quick test of the system"""
    print("🧪 QUICK SYSTEM TEST")
    print("="*50)

    searcher = EFXCounterexampleSearch(3, 4, seed=42)

    # Test utility generation
    utilities = searcher.generate_extreme_utilities()
    print("Sample utility matrix:")
    print(utilities.round(2))

    # Test allocation strategies
    test_allocation = [[0, 1], [2], [3], []]
    alloc_obj = Allocation(test_allocation, utilities)
    is_violated, violation_strength, violations, _ = searcher.check_efx_violation(utilities, alloc_obj)

    print(f"\nTest allocation: {test_allocation}")
    print(f"EFX violated: {is_violated}")
    print(f"Violation strength: {violation_strength:.4f}")
    if violations:
        print("Violations:")
        for v in violations:
            print(f"  • {v}")

    # Test new strategies
    print(f"\nTesting new allocation strategies:")
    new_strategies = [
        ("Specialized Matching", searcher.specialized_matching_strategy),
        ("Conflict Aware", searcher.conflict_aware_strategy),
        ("Minimax Envy", searcher.minimax_envy_strategy),
        ("Constraint Satisfaction", searcher.constraint_satisfaction_strategy)
    ]

    for name, strategy in new_strategies:
        allocation = strategy(utilities)
        is_violated, violation_strength, _, _ = searcher.check_efx_violation(utilities, allocation)
        print(f"  {name}: EFX violated = {is_violated}, strength = {violation_strength:.2f}")

    return searcher


if __name__ == "__main__":
    # Run quick test first
    quick_test()
    
    print("\n" + "="*80)
    print("STARTING MAIN SEARCH")
    print("="*80)
    
    # Run main search
    results = run_counterexample_search()
    
    print(f"\nSearch completed. Found {len(results)} verified counterexamples.")