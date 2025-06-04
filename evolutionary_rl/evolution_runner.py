from evolutionary_rl.counterexample_search import EFXCounterexampleSearch

def run_counterexample_search():
    print("🎯 AGGRESSIVE EFX COUNTEREXAMPLE SEARCH")
    print("="*80)

    test_configurations = [
        (4, 6),
        (4, 7),
        (5, 8),
        (6, 9),
    ]

    verified_counterexamples = []

    for num_players, num_items in test_configurations:
        print(f"\n{'='*60}")
        print(f"TESTING: {num_players} players, {num_items} items")
        print(f"{'='*60}")

        searcher = EFXCounterexampleSearch(num_players, num_items)
        candidates = searcher.evolutionary_search()

        if candidates:
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

    print(f"\n{'='*80}")
    print("FINAL RESULTS")
    print(f"{'='*80}")

    if verified_counterexamples:
        print(f"🎉 SUCCESS: {len(verified_counterexamples)} VERIFIED COUNTEREXAMPLES!")
        for i, ce in enumerate(verified_counterexamples, 1):
            config = ce['config']
            fitness = ce['candidate']['fitness']
            print(f"Counterexample {i}: {config[0]} players, {config[1]} items (fitness: {fitness:.2f})")
    else:
        print("❌ NO COUNTEREXAMPLES FOUND")

def quick_test():
    print("🧪 QUICK SYSTEM TEST")
    print("="*50)

    searcher = EFXCounterexampleSearch(3, 4, seed=42)
    utilities = searcher.generate_extreme_utilities()
    print("Sample utility matrix:")
    print(utilities.round(2))

    test_allocation = [[0, 1], [2], [3], []]
    is_violated, violation_strength, violations = searcher.check_efx_violation(utilities, test_allocation)

    print(f"\nTest allocation: {test_allocation}")
    print(f"EFX violated: {is_violated}")
    print(f"Violation strength: {violation_strength:.4f}")
    if violations:
        print("Violations:")
        for v in violations:
            print(f"  • {v}")

    print(f"\nTesting new allocation strategies:")
    new_strategies = [
        ("Specialized Matching", searcher.specialized_matching_strategy),
        ("Conflict Aware", searcher.conflict_aware_strategy),
        ("Minimax Envy", searcher.minimax_envy_strategy),
        ("Constraint Satisfaction", searcher.constraint_satisfaction_strategy)
    ]

    for name, strategy in new_strategies:
        allocation = strategy(utilities)
        is_violated, violation_strength, _ = searcher.check_efx_violation(utilities, allocation)
        print(f"  {name}: EFX violated = {is_violated}, strength = {violation_strength:.2f}")

    return searcher

if __name__ == "__main__":
    quick_test()
    print("\n" + "="*80)
    print("STARTING MAIN SEARCH")
    print("="*80)
    run_counterexample_search()
