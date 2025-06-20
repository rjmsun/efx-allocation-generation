#!/usr/bin/env python3
"""
Test script to verify imports work correctly from the notebooks directory.
"""

import sys
import os

# Add parent directory to Python path
sys.path.append(os.path.abspath('..'))

print("Current working directory:", os.getcwd())
print("Python path:", sys.path)

try:
    import efx
    print("✅ Successfully imported efx module")
    
    from efx import Allocation
    print("✅ Successfully imported Allocation class")
    
    import graphs
    print("✅ Successfully imported graphs module")
    
    from graphs import build_champion_graph, build_strong_envy_graph
    print("✅ Successfully imported graph functions")
    
    # Test creating an allocation
    test_valuations = [[1, 2], [3, 4]]
    test_bundles = [[0], [1]]
    allocation = Allocation(test_bundles, test_valuations)
    print("✅ Successfully created Allocation object")
    
except ImportError as e:
    print(f"❌ Import error: {e}")
except Exception as e:
    print(f"❌ Other error: {e}")

print("\nImport test completed!") 