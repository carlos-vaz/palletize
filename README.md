# Stackable Pallet Solver

## Usage
```bash
# compile
g++ src/* -I./inc/ -o main

# run
./main >> data.txt

# visualize
python3 plot.py data.txt
```

![alt text](https://github.com/carlos-vaz/palletize/blob/master/images/test1.png)

*Solution to sample problem. Colors indicate height (red = tallest, blue = shortest). SKU ID is the number in each box*


## Assumptions
1. Boxes cannot be rotated about any axis when placed

2. Pallet is big enough for any corner-placed boxes to never interfere with other corner-placed boxes

## Approach
### Step 1: Create a 4-corner support height
From tallest to shortest, find box heights that occur at least 4 times (using hashmap). Try to place each box in one of the 4 pallet corners, moving on to the next corner or box if not possible. For each height class, try to fit higher-volume boxes first.

### Step 2: Create list of all corners and their orientations
For each box shorter or equal to the max support height, starting from highest to lowest volume (to improve packing space efficiency), go through the list of *concave* corners and orientations until the box fits in one of them. Each corner can be in 1 of 10 geometric states, 6 of which are concave. Only concave corners are considered for placement to improve packing efficiency, since a box placed on a convex corner reduces the coalesence of available space (and a box placed into a corner such that it touches no concave corners can always be translated to a concave one, so there is no loss of optimality). 

### Step 3: Update affected corners and repeat
For each box placed, re-evaluate the orientations and existence of any corners in your list that the box touches.  

## Limitations
1. This algorithm never removes a box if it finds placing it leads to a suboptimal or incomplete placement solution. Any optimal solution would have to have a backtracking algorithm to search more possibilities. 

2. Not being able to rotate boxes (e.g. a width can become a height) means less possible configurations for
a 4-corner support height, and less possible placement options. 

3. In real life, a large box might span 2 or even 4 pallet corners, but this algorithm assumes 4 corners = 4 boxes


## Better approach / future work
Instead of randomly choosing concave corners and seeing if a box fits, we could have a heuristic to see which corners to try first to improve packing efficiency. We can calculate a list of locally maximum boxes (LMB), i.e. rectangles that cannot be expanded into any direction. We could keep these LMBs in an ordered linked list.

![alt text](https://github.com/carlos-vaz/palletize/blob/master/images/box_lmb.png)

*Locally Maximum Boxes (LMB) in pallet (there are 2 not shown)*

For each box (again, starting from the highest volume box available), we could try to find the smallest LMB that fits it. LMBs represent coalesced free space, and the largest LMBs are our most precious asset for packing large boxes. So if we take a large box and fit it into the smallest possible LMB, we would be optimizing for space. 

More work also needs to be done with the searching algorithm. Backtracking and rotating boxes need to be incorporated for a more optimal solution. The search algorithm can keep track of wasted volume percent (ratio between filled and unfilled volume in pallet stack) as it searches the tree of placement possibilities and select the configuration with the least waste. 