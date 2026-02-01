#include <vector>
#include <iostream>
#include "StackablePallet.hpp"

using namespace std;


int main() {
    StackablePallet pallet(1200, 800);

    pallet.addSkuToBacklog(0, 206, 198, 278, 6);
    pallet.addSkuToBacklog(1, 360, 300, 165, 2);
    pallet.addSkuToBacklog(2, 388, 280, 192, 1);
    pallet.addSkuToBacklog(3, 370, 298, 220, 1);

    pallet.solve();

    pallet.printSolution();

    return 0;
}