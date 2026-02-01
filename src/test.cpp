#include <vector>
#include <iostream>
#include "StackablePallet.hpp"

using namespace std;


int main() {
    // Create stackable pallet with dimensions 1200mm by 800mm
    StackablePallet pallet(1200, 800);

    // Add four different SKU classes (id, x,y,z, quantity)
    pallet.addSkuToBacklog(0, 206, 198, 278, 6);
    pallet.addSkuToBacklog(1, 360, 300, 165, 2);
    pallet.addSkuToBacklog(2, 388, 280, 192, 1);
    pallet.addSkuToBacklog(3, 370, 298, 220, 1);

    // Try to place as many boxes as possible in stackable configuration
    pallet.solve();

    // Print output to stdout (can be plotted with plot.py)
    pallet.printSolution();

    return 0;
}