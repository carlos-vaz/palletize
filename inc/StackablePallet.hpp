#include <vector>
#include <stdint.h>

using namespace std;

typedef struct {
    int l;
    int w;
    int h;
    int amt;
    int id;
    void* next;
    void* nextSameHeight;
} skuLL_t;

typedef struct { // placed box linked list
    skuLL_t* sku;
    int origin[2];
    void* next;
} boxLocationLL_t;

typedef struct { // concave corners linked list
    int pos[2]; 
    int orientation;
    void* next;
} cornerLL_t;

typedef struct { // local maximum (fully bounded, unexpandable) available space rectangle linked list
    int origin[2];
    int dims[2];
    void *next;
} localMaxRectangleLL_t;

// ########## SIMPLE ##########

typedef struct {
    int dims[3];
    int origin[2]; // [-1,..] if not yet placed
    int id;
} box_t;

typedef struct {
    int pos[2];
    bool isConcave;
    uint8_t orientation;
    bool active; // non-pallet corners are deactivated durring 4-corner support building stage
} corner_t;


class StackablePallet {
    public:
        StackablePallet(int pX, int pY);
        ~StackablePallet() = default;
        void testSetup();
        bool placeBox(box_t &box);
        void addSkuToBacklog(int id, int l, int w, int h, int quantity);
        void solve();


        void printStuff();
        void printSolution();

    private:
        int pX;
        int pY;
        int maxHeight; // -1 until support configuration is found

        //boxLocationLL_t *boxListHead; // linked list of all placed boxes
        skuLL_t *volumeOrderedListHead; // linked list of all available SKUs ordered by volume
        void placeSku(skuLL_t *sku);
        void insertAvailSku(skuLL_t *skull);


        bool overlaps(box_t &box1, box_t &box2);
        int placeBoxOnConcaveCorner(box_t &box, corner_t &corner);
        bool updateCorner(box_t &box, corner_t &corner);

        vector<box_t> placements;
        vector<corner_t> corners; 
};