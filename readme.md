# Advanced Algorithm Design Assignment: The Pi Bin Packing Challenge

## 1. Problem Context & Background

The *Bin Packing Problem* is a classic combinatorial optimization problem and a foundational member of the class of *NP-hard* problems. In this assignment, you will build an optimization engine to solve a scaled-up, non-enumerable variant of this problem.

To ensure the dataset is completely deterministic, pseudo-random, and highly irregular, the item sizes are derived directly from the fractional decimal digits of the mathematical constant $\pi$.

---

## 2. Objective

Given an unlimited supply of identical bins, each with a fixed maximum capacity $C = 1.0$, and a large sequence of item weights $W = \{w_1, w_2, \dots, w_n\}$, your goal is to distribute *all* items into the *minimum possible number of bins* without any single bin exceeding capacity $C$.

Mathematically, for each item $i$ and bin $j$:


$$\sum_{i \in \text{Bin}_j} w_i \le 1.0$$

---

## 3. The Dataset Specification (Non-Enumerable Scale)

Because the problem is NP-hard, a brute-force approach ($O(2^n)$ or $O(n!)$) will freeze your system if $n$ is sufficiently large. For this assignment, you will scale the problem to *$n = 10,000$ items*.

### Data Generation Rule:

1. Extract the first *40,000 digits* of $\pi$ after the decimal point: 141592653589...
2. Group these digits into consecutive *4-digit blocks* (e.g., 1415, 9265, 3589, 7932).
3. Normalize each block into a floating-point value between $0.0001$ and $0.9999$ by dividing the 4-digit integer by $10,000$.
4. This yields a precise array of $10,000$ weights:

$$W = [0.1415, 0.9265, 0.3589, 0.7932, \dots]$$



---

## 4. Assignment Requirements & Architecture

You are required to build a structured pipeline containing three distinct algorithmic layers. Do not use external commercial solvers (like Gurobi or CPLEX). Everything must be coded from scratch.

### Task 1: Establish the Lower Bound

Before optimizing, compute the theoretical absolute minimum number of bins required. This acts as your baseline benchmark.

* *Formula:* 
$$\text{Lower Bound} = \lceil \sum_{i=1}^{n} w_i \rceil$$


* Note: Due to packing inefficiencies, this bound is often impossible to reach, but it serves as your ultimate point of comparison.

### Task 2: Constructive Heuristics (The Baseline)

Implement the following two deterministic greedy algorithms:

1. *First-Fit Decreasing (FFD):* * Sort the 10,000 items in descending order.
* For each item, scan the existing bins sequentially from the first one created. Place the item into the first bin that has enough residual space. If none can hold it, open a new bin.


2. *Best-Fit Decreasing (BFD):* * Sort the items in descending order.
* For each item, place it into the bin that has the tightest fit (i.e., the bin that will have the absolute minimum remaining space after the item is added). If no bin can hold it, open a new bin.



### Task 3: Metaheuristic / Approximation (The Core Challenge)

Because the constructive heuristics are local and short-sighted, you must implement an iterative approximation metaheuristic to shake up the distribution and find a tighter packing.

Choose *one* of the following architectures to implement:

* *Option A: Simulated Annealing (SA)*
* Define a state as a valid configuration of bins.
* Define a neighbor transition (e.g., pick two random items from different bins and swap them, or move a small item to a bin with matching residual capacity).
* Design a fitness function that penalizes unevenly filled bins or rewards perfectly packed bins.


* *Option B: Genetic Algorithm (GA)*
* Treat a chromosome as an array representing bin assignments.
* Implement an order-based crossover operator to prevent invalid or overfilled bins during reproduction.



---

## 5. Deliverables & Evaluation Criteria

Your final code execution must print a clean execution report to the console mapping out performance metrics.

### Expected Console Output Format:

text
============================================================
THE PI BIN PACKING OPTIMIZATION REPORT
============================================================
Total Items Generated : 10,000 (Using 4-digit Pi chunks)
Theoretical Minimum   : XXXX Bins

[ALGORITHM RESULTS]
1. First-Fit Decreasing (FFD)
   - Bins Used        : XXXX
   - Execution Time   : XX.XX ms
   - Efficiency Gap   : +X.XX% vs Lower Bound

2. Best-Fit Decreasing (BFD)
   - Bins Used        : XXXX
   - Execution Time   : XX.XX ms
   - Efficiency Gap   : +X.XX% vs Lower Bound

3. Metaheuristic Engine (Simulated Annealing / GA)
   - Bins Used        : XXXX
   - Iterations Run   : XXXXX
   - Final Gap        : +X.XX% vs Lower Bound
============================================================



### Grading Rubric:

* *Correctness (30%):* Bins must never exceed $1.0$ capacity. All 10,000 items must be successfully assigned.
* *Algorithm Design (40%):* Correct, clean execution of both FFD/BFD heuristics and a working iterative cooling or evolutionary schedule for the metaheuristic.
* *Optimization Performance (30%):* The degree to which your metaheuristic is capable of improving upon the greedy baseline architectures.


## 6. Addendum: Competitive Leaderboard & Evaluation Metric
To facilitate a competitive student leaderboard, raw bin counts alone are insufficient, as many top-tier implementations will achieve identical bin counts. To break these ties and reward truly optimized, tightly packed bin configurations, submissions will be ranked using a continuous loss function based on *Falkenauer's metric*.
Your goal is to *minimize* the following total loss score L:
### Metric Breakdown
 * *K*: The total number of bins used in your solution (primary objective).
 * *F_j*: The total accumulated weight (fill level) of bin j, where a valid bin satisfies 0 < F_j \le 1.0.
 * *The Tie-Breaker Term (\frac{\sum F_j^2}{K})*: Squaring the fill levels mathematically rewards highly unequal distributions. For example, a solution that packs two bins at 0.9 and 0.1 yields a higher squared sum (0.81 + 0.01 = 0.82) than one that packs them evenly at 0.5 and 0.5 (0.25 + 0.25 = 0.50). This forces the metaheuristic to pack bins to the absolute brim to maximize the deducted fraction and lower the total loss.
 * *\text{Penalty}*: A severe barrier to ensure that invalid or overfilled solutions cannot claim a top spot on the leaderboard.
> *Why This Works:* Because 0 < F_j \le 1.0, the average squared fullness term is strictly bounded between 0 and 1. Therefore, any valid solution utilizing 5,000 bins will always achieve a lower (better) loss than any valid solution utilizing 5,001 bins. Tie-breaking happens completely in the decimal spaces.
> 
### Handling Constraints (The Penalty Function)
If your algorithm accidentally overflows a bin, your solution is technically invalid. To prevent invalid configurations from corrupting the leaderboard while still providing your metaheuristic with a mathematical gradient to steer back into valid territory, the penalty is calculated as:
If all bins successfully respect the maximum capacity C = 1.0, the penalty drops to 0.
### Reference Python Implementation
Integrate the following function into your final evaluation pipeline. Your metaheuristic engine should optimize against this exact score.
python
def calculate_competitive_loss(bins_list):
    """
    Computes the competitive leaderboard loss for the assignment addendum.
    
    Parameters:
    bins_list (list of lists): A list where each sublist contains the weights 
                               assigned to that specific bin.
                               e.g., [[0.9265, 0.05], [0.1415, 0.3589, 0.4]]
                               
    Returns:
    float: The final calculated loss rounded to 6 decimal places.
    """
    K = len(bins_list)
    if K == 0:
        return float('inf')
        
    sum_squared_fills = 0.0
    total_penalty = 0.0
    
    for r_bin in bins_list:
        fill = sum(r_bin)
        sum_squared_fills += fill ** 2
        
        # Apply a severe penalty if a bin exceeds the 1.0 capacity limit
        if fill > 1.0:
            total_penalty += (fill - 1.0) * 1000000
            
    # Compute the Falkenauer-based minimization loss
    tie_breaker = sum_squared_fills / K
    loss = K - tie_breaker + total_penalty
    
    return round(loss, 6)
