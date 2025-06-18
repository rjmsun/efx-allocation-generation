#ifndef ALLOCATION_HPP
#define ALLOCATION_HPP

#include <vector>

using Allocation = std::vector<std::vector<int>>;
using Utilities = std::vector<std::vector<int>>;

// total utiliy that an agent assigns to bundle
int total_value(const Utilities& utils, int agent, const std::vector<int>& bundle);

// total utiliy that an agent assigns to bundle after removal
int value_excluding_least(const Utilities& utils, int agent, const std::vector<int>& bundle);

// checks EFX
bool isEFX(const Allocation& allocation, const Utilities& utilities);

// brute force
bool hasEFXAllocation(const Utilities& utilities, int num_items);

#endif