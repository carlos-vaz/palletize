#include <vector>
#include <stdint.h>

using namespace std;

/* SKU Linked List type */
typedef struct {
    int l;
    int w;
    int h;
    int amt; // number of boxes in this SKU class
    int id;
    void* next; // list of skuLL_t in decreasing volume order
    void* nextSameHeight; // list of skuLL_t of same height
} skuLL_t;

/* Box type */
typedef struct {
    int dims[3];
    int origin[2]; // [-1,..] if not yet placed
    int id;
} box_t;

/* Corner type */
typedef struct {
    int pos[2];
    bool isConcave;
    uint8_t orientation;
    bool active; // non-pallet corners are deactivated durring 4-corner support building stage
} corner_t;


class StackablePallet {
    public:
        /* Initializer
         * PARAMS:
         *      palletLength - length of pallet
         *      palletWidth  - width of pallet
         */
        StackablePallet(int palletLength, int palletWidth);
        ~StackablePallet() = default;

        /* Tries to accomodate a box into the current pallet configuration
         * PARAMS:
         *      box - reference to an unplaced box_t object
         * RETURNS: true if placed, false otherwise
         */
        bool placeBox(box_t &box);

        /* Consider some quantity of boxes of an SKU class for placement on pallet
         * PARAMS:
         *      id -       integer identifier of SKU class
         *      l, w, h -  dimensions of SKU class
         *      quantity - number of boxes in class
         */
        void addSkuToBacklog(int id, int l, int w, int h, int quantity);

        /* Place as many boxes from the backlog into the pallet in such a way
         * that the four corners have boxes of equal height (for stackability).
         * Box placements are stored in internal 'placements' vector
         */
        void solve();

        /* Print solutions to stdout in a way that plot.py can parse to create a 
         * plot visualization.
         */
        void printSolution();


    private:
        int pX; // pallet length
        int pY; // pallet width
        int maxHeight; // -1 until support configuration is found
        skuLL_t *volumeOrderedListHead; // linked list of all available SKUs ordered by volume
        vector<box_t> placements; // placed boxes
        vector<corner_t> corners; // vector of all corners

        void placeSku(skuLL_t *sku);
        bool overlaps(box_t &box1, box_t &box2);
        int placeBoxOnConcaveCorner(box_t &box, corner_t &corner);
        bool updateCorner(box_t &box, corner_t &corner);
        void printStuff(); // debug only
};