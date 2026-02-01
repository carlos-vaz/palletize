# Stackable Pallet Solver

## Usage
```bash
# compile
g++ *.cpp -o main

# run
./main >> data.txt

# visualize
python3 plot.py data.txt
```

## Assumptions
1. Boxes cannot be rotated about any axis when placed
2. Pallet is big enough for any corner-placed boxes to never interfere with other corner-placed boxes
3. 

## Approach
### Step 1: Create a 4-corner support height
From tallest to shortest, find box heights that occur at least 4 times (using hashmap). Try to place each box in one of the 4 pallet corners, moving on to the next corner or box if not possible. For each height class, try to fit higher-volume boxes first.

### Step 2: Create list of all corners and their orientations
For each box shorter or equal to the max support height, starting from highest to lowest volume (to improve packing space efficiency), go through the list of corners and orientations until the box fits in one of them. (A corner can have any of 4 orientations available for placement, so a corner can have one of 2^4 = 16 geometric states (10 of which are valid))

### Step 3: 

## Limitations
1. This algorithm never removes a box if it finds placing it leads to a suboptimal or incomplete placement solution. Any optimal solution would have to have a backtracking algorithm to search more possibilities. 
2. Not being able to rotate boxes (e.g. a width can become a height) means less possible configurations for
a 4-corner support height, and less possible placement options. 

## Better approach / future work

