# Advanced Algorithm Design Assignment: The Corrupted Pi Bin Packing Challenge

## 1. Problem Context & Background
In the study of combinatorial optimization, the *Bin Packing Problem* is a foundational NP-hard dilemma. However, real-world engineering rarely presents problems in isolation. In logistics, certain hazardous materials cannot be transported in the same vehicle. In cloud infrastructure, highly volatile virtual machines (VMs) cannot be hosted on the same physical blade due to hardware interference or security isolation protocols.

This assignment introduces the **Bin Packing Problem with Conflicts (BPWP)**. To solve it, your optimization engine must simultaneously handle a packing constraints problem (Knapsack variant) and an independent-set partitioning problem (Graph Coloring variant). 

---

## 2. Objective
Given an infinite supply of identical bins, each with a fixed maximum capacity $C = 1.0$, and a highly irregular sequence of item weights $W = \{w_1, w_2, \dots, w_n\}$, your goal is to distribute *all* items into the **minimum possible number of bins** while strictly respecting two independent dimensions of constraints:

1. **Capacity Constraint:** The cumulative weight of all items assigned to any single bin $j$ must never exceed $1.0$.
$$\sum_{i \in \text{Bin}_j} w_i \le 1.0$$
2. **Conflict Constraint:** If an edge exists between Item $A$ and Item $B$ in a global Conflict Graph ($G$), those two items **cannot** occupy the same bin, regardless of how much residual capacity that bin has left.

---

## 3. Adversarial Dataset Specification
To ensure the dataset is completely deterministic, pseudo-random, yet highly irregular and non-trivial for greedy heuristics, all properties are derived directly from the fractional decimal digits of the mathematical constant $\pi$. 

The scale of this problem is set to **$n = 10,000$ items** and exactly **$M = 15,000$ conflict edges**.

### Step 1: Item Weights Generation (Heavy-Tailed Distribution)
1. Extract the first *40,000 digits* of $\pi$ after the decimal point (starting: `141592653589...`).
2. Group these digits into consecutive *4-digit blocks* (e.g., `1415`, `9265`, `3589`, `7932`).
3. Normalize each block into a floating-point baseline between $0.0001$ and $0.9999$ by dividing the 4-digit integer by $10,000$.
4. **The Adversarial Skew:** To break uniform symmetry and simulate a brutal, heavy-tailed payload distribution, square the baseline value:
$$w_i = \left(\frac{\text{Block}_i}{10000}\right)^2$$
This yields your precise array of 10,000 weights: $W = [0.0200, 0.8584, 0.1288, 0.6292, \dots]$.

### Step 2: Conflict Graph Generation
To generate the $15,000$ undirected conflict edges without external file dependencies:
1. Advance to the *next* sequence of digits in $\pi$, starting precisely at **digit 40,001**.
2. Group these digits into *5-digit blocks*.
3. Map each block to an item index using the modulo operator: $\text{Index} = \text{Block} \pmod{10000}$.
4. Take pairs of consecutive 5-digit blocks to form an undirected edge $(u, v)$. 
5. *Self-loops ($u = v$) or duplicate edges must be discarded.* Continue processing 5-digit blocks sequentially until your graph contains exactly **15,000 unique conflict edges**.

---

## 4. Assignment Requirements & Architecture
You are required to build a modular optimization pipeline written entirely from scratch. External commercial solvers (e.g., Gurobi, CPLEX, SCIP) are strictly prohibited.

### Task 1: Establish the Lower Bound
Compute the theoretical absolute minimum number of bins required based strictly on mass.
$$\text{Lower Bound} = \lceil \sum_{i=1}^{n} w_i \rceil$$
*Note: Due to the severe constraints added by the 15,000 conflict edges, this physical bound will likely be mathematically impossible to hit, but it remains your ultimate point of comparison.*

### Task 2: Conflict-Aware Heuristics (The Baseline)
Implement modified, conflict-aware variations of the classic greedy packing algorithms:

1. **Conflict-Aware First-Fit Decreasing (C-FFD):**
   * Sorting by weight alone is no longer optimal. You must design a **Hybrid Priority Metric** to sort your items before packing. For example, sort by a combination of weight and vertex degree: $\text{Priority}_i = \alpha \cdot w_i + \beta \cdot \text{Degree}(i)$.
   * Sequentially process items down your sorted priority list. Place the item into the first existing bin that has both *sufficient residual weight capacity* AND *zero conflict violations* with items already inside that bin. If no bin fits, open a new one.
2. **Conflict-Aware Best-Fit Decreasing (C-BFD):**
   * Use your hybrid sorted priority list. Place each item into the valid bin that yields the *tightest remaining weight capacity* while maintaining absolute conflict isolation. If no existing bin can legally host it, open a new bin.

### Task 3: Metaheuristic Engine (The Core Challenge)
Because greedy heuristics cannot foresee downstream bottlenecks caused by complex graph topologies, you must implement an iterative approximation metaheuristic to reshuffle assignments dynamically.

Choose **one** of the following architectures:
* **Option A: Simulated Annealing (SA)**
  * *State:* A complete, indexed allocation mapping all 10,000 items to active bins.
  * *Neighborhood Move:* Implement intelligent transitions. Random swapping will consistently trigger conflict penalties. Create operators that explicitly check adjacent vertices or shift conflicting items to safe bins.
  * *Cooling Schedule:* Design an appropriate geometric or linear cooling schedule to avoid getting trapped in local minima.
* **Option B: Genetic Algorithm (GA)**
  * *Chromosome:* An array of size 10,000 where the value at index $i$ denotes the bin assigned to item $i$.
  * *Evolutionary Operators:* Standard random crossovers will create massive numbers of invalid, conflicting chromosomes. You must engineer specialized **Conflict-Aware Crossover/Mutation Operators** or a dedicated **Mendelian Repair Heuristic** that evicts conflicting nodes post-reproduction and re-homes them systematically.

---

## 5. Evaluation & The Competitive Leaderboard
To break ties on identical bin counts and reward truly exceptional optimization, submissions will be ranked using a continuous loss function based on *Falkenauer's metric*, heavily scaled with severe economic barriers for hard constraint violations.

### The Objective Loss Function ($L$):
$$L = K - \frac{\sum_{j=1}^{K} F_j^2}{K} + \text{Penalty}_{\text{Capacity}} + \text{Penalty}_{\text{Conflict}}$$

Where:
* $K$ is the total number of bins used.
* $F_j$ is the accumulated weight (fill level) of bin $j$. Squaring $F_j$ mathematically rewards highly unequal, tightly packed distributions over soft, averaged distributions.
* $\text{Penalty}_{\text{Capacity}}$ heavily penalizes any bin where $F_j > 1.0$.
* $\text{Penalty}_{\text{Conflict}}$ applies a catastrophic cost to every instance where a pair of conflicting items shares a bin.

### Mandatory Python Evaluation Function
Your optimization pipeline must evaluate state fitness against this exact function:

```python
def calculate_competitive_loss(bins_list, conflict_graph, global_weights_array):
    """
    Computes the competitive leaderboard loss for the BPWP assignment.
    
    Parameters:
    bins_list (list of lists): A list of bins, where each sublist contains the 
                               integer IDs (0-9999) of items assigned to it.
                               e.g., [[0, 4, 9], [1, 2], [3, 5, 6]]
    conflict_graph (dict): Adjacency list mapping item ID -> set of conflicted item IDs.
                           e.g., { 0: {12, 450}, 1: set(), ... }
    global_weights_array (list): The complete 10,000 array of heavy-tailed weights.
    """
    K = len(bins_list)
    if K == 0:
        return float('inf')
        
    sum_squared_fills = 0.0
    capacity_penalty = 0.0
    conflict_penalty = 0.0
    
    for r_bin in bins_list:
        # 1. Compute Fill Level and Capacity Penalties
        fill = sum(global_weights_array[item_id] for item_id in r_bin)
        sum_squared_fills += fill ** 2
        
        if fill > 1.0:
            capacity_penalty += (fill - 1.0) * 1000000
            
        # 2. Compute Graph Conflict Penalties
        bin_items = set(r_bin)
        for item_id in r_bin:
            # Intersection finds how many items inside this bin conflict with item_id
            conflicts_in_bin = bin_items.intersection(conflict_graph.get(item_id, set()))
            # Each conflict pair (u, v) will be hit twice, generating a massive penalty
            conflict_penalty += len(conflicts_in_bin) * 500000 

    # Compute Falkenauer optimization metric
    tie_breaker = sum_squared_fills / K
    loss = K - tie_breaker + capacity_penalty + conflict_penalty
    
    return round(loss, 6)