/*
SKU A: [206, 198, 278] -> 6 cases
SKU B: [360, 300, 165] -> 2 cases
SKU C: [388, 280, 192] -> 1 case
SKU D: [370, 298, 220] -> 1 case

Pallet size = 1200 x 800 x 145

aditi.pandya@mujin-corp.com
Github username: zureallyis
*/

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
    pallet.printStuff();

    pallet.solve();
    pallet.printStuff();

    cout << endl;

    pallet.printSolution();

    return 0;
}