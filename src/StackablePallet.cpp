#include "StackablePallet.hpp"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <map>
#include <set>

/* Initializer
 * PARAMS:
 *      palletLength - length of pallet
 *      palletWidth  - width of pallet
 */
StackablePallet::StackablePallet(int xdim, int ydim) {
    this->pX = xdim;
    this->pY = ydim;
    this->maxHeight = -1;
    volumeOrderedListHead = NULL;
    // place initial 4 available corners into list
    this->corners.push_back({.pos={0,0}, .isConcave=true, .orientation=1, .active=true});
    this->corners.push_back({.pos={xdim,0}, .isConcave=true, .orientation=2, .active=true});
    this->corners.push_back({.pos={0,ydim}, .isConcave=true, .orientation=4, .active=true});
    this->corners.push_back({.pos={xdim,ydim}, .isConcave=true, .orientation=8, .active=true});
}

/* Consider some quantity of boxes of an SKU class for placement on pallet
 * PARAMS:
 *      id -       integer identifier of SKU class
 *      l, w, h -  dimensions of SKU class
 *      quantity - number of boxes in class
 */
void StackablePallet::addSkuToBacklog(int id, int l, int w, int h, int quantity) {
    skuLL_t *skull = new skuLL_t;
    skull->id = id;
    skull->amt = quantity;
    skull->l = l;
    skull->w = w;
    skull->h = h;
    skull->next = NULL;

    if(this->volumeOrderedListHead==NULL) {
        volumeOrderedListHead = skull;
    }
    else {
        skuLL_t *curr = this->volumeOrderedListHead;
        skuLL_t *prev = NULL;
        bool inserted = false;
        while(curr != NULL) {
            if(l*w*h >= (curr->l)*(curr->w)*(curr->h)) {
                // insert here
                if(prev==NULL) {
                    volumeOrderedListHead = skull; 
                }
                else {
                    prev->next = skull;
                }
                skull->next = curr;
                inserted = true;
            }
            prev = curr;
            curr = (skuLL_t*)curr->next;
            if(inserted) {
                break;
            }
        }
        if(!inserted) {
            prev->next = skull;
        }
    }
}

/* Place as many boxes from the backlog into the pallet in such a way
 * that the four corners have boxes of equal height (for stackability).
 * Box placements are stored in internal 'placements' vector
 */
void StackablePallet::solve() {
    // first, we must build a 4-corner same-height support configuration...
    // build height maps
    map<int, skuLL_t*> heightToSkuMap;
    map<int, int> heightToFrequencyMap;
    skuLL_t *curr = this->volumeOrderedListHead;
    while(curr != NULL) {
        if(heightToSkuMap.count(curr->h) != 0) {
            // add to linked list of same-height SKUs
            skuLL_t *csh = heightToSkuMap[curr->h];
            while(csh != NULL) {
                csh = (skuLL_t*)csh->nextSameHeight;
            }
            csh->nextSameHeight = curr;
            heightToFrequencyMap[curr->h] += curr->amt;
        }
        else {
            heightToSkuMap.insert({curr->h,curr});
            heightToFrequencyMap[curr->h] = curr->amt;
        }
        curr->nextSameHeight = NULL;
        curr = (skuLL_t*)curr->next;
    }

    // search for box heights of frequency 4+ from tallest to shortest
    vector<int> heights;
    for(auto const &h : heightToFrequencyMap) {
        heights.push_back(h.first);
    }
    sort(heights.begin(), heights.end(), std::greater<>());
    int supportsPlaced = 0;
    for(int h : heights) {
        if(heightToFrequencyMap[h] >= 4) {
            // we can try this height configuration
            curr = heightToSkuMap[h];
            while(curr != NULL) {
                if(curr->amt != 0) {
                    box_t box = {.dims={curr->l,curr->w,curr->h}, .origin={-1,0}, .id=curr->id};
                    if(placeBox(box)) {
                        supportsPlaced++;
                        heightToFrequencyMap[h]--;
                        curr->amt--;
                        //cout << "SUPPORT CONFIG --> PLACED SKU "<<curr->id<<" at pos=("<<box.origin[0]<<","<<box.origin[1]<<")"<<endl;
                        if(supportsPlaced == 4) {
                            // done with support building
                            this->maxHeight = h;
                            break;
                        }
                    }
                    else {
                        curr = (skuLL_t*)curr->nextSameHeight;
                    }
                }
                else {
                    curr = (skuLL_t*)curr->nextSameHeight;
                }
            }
            if(supportsPlaced == 4) {
                // done
                break;
            }
        }
    }

    // Done with 4-corner support configuration
    // Next, fill in remaining boxes

    curr = this->volumeOrderedListHead;
    while(curr != NULL) {
        if(curr->amt != 0) {
            // try to fit the next highest volume SKU box
            box_t box = {.dims={curr->l,curr->w,curr->h}, .origin={-1,0}, .id=curr->id};
            bool placed = placeBox(box);
            if(placed) {
                //cout << "PLACED SKU "<<curr->id<<" at pos=("<<box.origin[0]<<","<<box.origin[1]<<")"<<endl;
                // decrement SKU quantity
                curr->amt--;
                // next, try to place another of the same SKU type
            }
            else {
                curr = (skuLL_t*)curr->next;
            }
        }
        else {
            curr = (skuLL_t*)curr->next;
        }
    }
}

/* Print pallet placement solution to stdout (visualized with plot.py) */
void StackablePallet::printSolution() {
    cout << "pallet_dim=" << this->pX << "," << this->pY << endl;
    for(box_t box : this->placements) {
        cout << "id="<<box.id<<" origin="<<box.origin[0]<<","<<box.origin[1]<< \
                " dims="<<box.dims[0]<<","<<box.dims[1]<<","<<box.dims[2]<<endl;
    }
}


// ############### INTERNAL HELPER FUNCTIONS ###############

/* Returns true iff there is any overlap among two placed boxes */
bool StackablePallet::overlaps(box_t &box1, box_t &box2) {
    // determine if there is any x-axis overlap
    bool xOverlap = (box1.origin[0] < box2.origin[0]) && (box2.origin[0] < box1.origin[0]+box1.dims[0]);
    xOverlap |= (box1.origin[0] < box2.origin[0]+box2.dims[0]) && \
                (box2.origin[0]+box2.dims[0] < box1.origin[0]+box1.dims[0]);
    xOverlap |= (box1.origin[0] == box2.origin[0]);
    if(!xOverlap) {
        return false;
    }
    // determine if there is any y-axis overlap
    bool yOverlap = (box1.origin[1] < box2.origin[1]) && (box2.origin[1] < box1.origin[1]+box1.dims[1]);
    yOverlap |= (box1.origin[1] < box2.origin[1]+box2.dims[1]) && \
                (box2.origin[1]+box2.dims[1] < box1.origin[1]+box1.dims[1]);
    yOverlap |= (box1.origin[1] == box2.origin[1]);
    if(!yOverlap) {
        return false;
    }
    // there is box collision only if there is both x and y axis overlap
    return true;
}

/* Place box on first of the available orientations of a concave corner where the box fits. 
 * Updates box and corner references if a fit is found. 
 */
int StackablePallet::placeBoxOnConcaveCorner(box_t &box, corner_t &corner) {
    // try to place in each available orientation
    uint8_t oShadow = corner.orientation;
    for(int i=0; i<4; i++) {
        if(oShadow&1) {
            // Orientation is available, try to fit the box
            switch(i) {
                case 0:
                    box.origin[0] = corner.pos[0];
                    box.origin[1] = corner.pos[1];
                    break; 
                case 1:
                    box.origin[0] = corner.pos[0]-box.dims[0];
                    box.origin[1] = corner.pos[1];
                    break;
                case 2:
                    box.origin[0] = corner.pos[0];
                    box.origin[1] = corner.pos[1]-box.dims[1];
                    break;
                case 3:
                    box.origin[0] = corner.pos[0]-box.dims[0];
                    box.origin[1] = corner.pos[1]-box.dims[1];
                    break;
            }
            // check if box fits in pallet boundaries
            if((box.origin[0]+box.dims[0]>this->pX) || (box.origin[1]+box.dims[1]>this->pY) || \
               (box.origin[0]<0) || (box.origin[1]<0)) {
                // does not fit in pallet
                box.origin[0] = -1;
                return -1;
            }

            bool collision = false;
            for(box_t pbox : this->placements) {
                if(overlaps(pbox, box)) {
                    collision = true;
                    break;
                }
            }
            if(!collision) {
                // box fits in this corner orientation
                // remove corner available orientation
                corner.orientation &= ~(1<<i);
                return i;
            }
        }
        oShadow >>= 1;
    }
    // could not fit in any corner orientation
    box.origin[0] = -1;
    return -1;
}

/* Update a corner's available orientations and concavity parameter if a specified 
 * box touches the corner
 */
bool StackablePallet::updateCorner(box_t &box, corner_t &corner) {
    // if box does not pass through corner, corner is unaffected (do nothing)
    bool affected = (box.origin[0] <= corner.pos[0]) && (corner.pos[0] <= box.origin[0]+box.dims[0]) && \
                    (box.origin[1] <= corner.pos[1]) && (corner.pos[1] <= box.origin[1]+box.dims[1]);
    if(!affected) {
        return (corner.orientation==0); // should be false
    }
    // remove the corner's available orientation destroyed by adding the box
    if(box.origin[0]+box.dims[0]>corner.pos[0] && box.origin[1]+box.dims[1]>corner.pos[1]) {
        corner.orientation &= ~(1<<0);
    }
    if(box.origin[0]<corner.pos[0] && box.origin[1]+box.dims[1]>corner.pos[1]) {
        corner.orientation &= ~(1<<1);
    }
    if(box.origin[0]+box.dims[0]>corner.pos[0] && box.origin[1]<corner.pos[1]) {
        corner.orientation &= ~(1<<2);
    }
    if(box.origin[0]<corner.pos[0] && box.origin[1]<corner.pos[1]) {
        corner.orientation &= ~(1<<3);
    }

    // remove orientations obstructed by pallet boundaries
    if(corner.pos[0]==0) {
        corner.orientation &= ~(1<<1);
        corner.orientation &= ~(1<<3);
    }
    if(corner.pos[0]==this->pX) {
        corner.orientation &= ~(1<<0);
        corner.orientation &= ~(1<<2);
    }
    if(corner.pos[1]==0) {
        corner.orientation &= ~(1<<2);
        corner.orientation &= ~(1<<3);
    }
    if(corner.pos[1]==this->pY) {
        corner.orientation &= ~(1<<0);
        corner.orientation &= ~(1<<1);
    }

    // update concavity parameter
    corner.isConcave = true;
    if((corner.orientation&(1<<0)!=0 && corner.orientation&(1<<1)!=0) || \
       (corner.orientation&(1<<0)!=0 && corner.orientation&(1<<2)!=0) || \
       (corner.orientation&(1<<3)!=0 && corner.orientation&(1<<2)!=0) || \
       (corner.orientation&(1<<1)!=0 && corner.orientation&(1<<3)!=0)) {
        corner.isConcave = false;
    }

    return (corner.orientation==0);
}


/* Place box on the first available concave corner that accomodates it
 * PARAMS:
 *      box - reference to an unplaced box_t object
 * RETURNS: true if placed, false otherwise
 */
bool StackablePallet::placeBox(box_t &box) {
    int orientationPlaced = 0;
    bool placed = false;

    if(box.dims[2]>this->maxHeight && this->maxHeight!=-1) {
        // this box is too tall
        return false;
    }

    // test concave corners, place box in first one that accomodates it
    for(corner_t &cc : this->corners) {
        if(!cc.isConcave) {
            continue;
        }
        if(!cc.active && this->maxHeight==-1) {
            // we're still in 4-corner support building stage, and this is not a pallet corner!
            continue;
        }
        orientationPlaced = placeBoxOnConcaveCorner(box, cc); // updates box origin and corner available orientations
        if(orientationPlaced >= 0) { 
            // no collision in one of the corner orientations!
            // add to list of placed boxes
            this->placements.push_back(box);
            placed = true;
            break;
        }
    }

    if(!placed) {
        return false;
    }

    // add up to 3 new corners (with all orientations available, this will be corrected in the next step)
    vector<corner_t> newCorners;
    switch(orientationPlaced) {
        case 0: 
            newCorners.push_back({.pos={box.origin[0]+box.dims[0],box.origin[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0]+box.dims[0],box.origin[1]+box.dims[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0],box.origin[1]+box.dims[1]}, \
                                        .isConcave=false, .orientation=0xff});
            break;
        case 1: 
            newCorners.push_back({.pos={box.origin[0],box.origin[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0]+box.dims[0],box.origin[1]+box.dims[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0],box.origin[1]+box.dims[1]}, \
                                        .isConcave=false, .orientation=0xff});
            break;
        case 2: 
            newCorners.push_back({.pos={box.origin[0],box.origin[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0]+box.dims[0],box.origin[1]+box.dims[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0]+box.dims[0],box.origin[1]}, \
                                        .isConcave=false, .orientation=0xff});     
            break;
        case 3: 
            newCorners.push_back({.pos={box.origin[0],box.origin[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0]+box.dims[0],box.origin[1]}, \
                                        .isConcave=false, .orientation=0xff});
            newCorners.push_back({.pos={box.origin[0],box.origin[1]+box.dims[1]}, \
                                        .isConcave=false, .orientation=0xff});
            break;
    }
    for(corner_t nc : newCorners) {
        bool foundMatching = false;
        for(corner_t c : this->corners) {
            if(c.pos[0]==nc.pos[0] && c.pos[1]==nc.pos[1]) {
                foundMatching = true;
                break;
            }
        }
        if(!foundMatching) {
            //cout << "adding TOTALLY NEW CORNER = ("<<nc.pos[0]<<","<<nc.pos[1]<<")"<<endl;
            this->corners.push_back(nc);
        }
    }

    // update all corner values if affected
    if(box.origin[0] != -1) {
        vector<int> cornersToRemove;
        for(int i=0; i<corners.size(); i++) {
            if(updateCorner(box, corners[i])) {
                // add corner to remove list
                //cout << "removing corner @ (" << corners[i].pos[0]<<","<<corners[i].pos[1]<<")"<<endl;
                cornersToRemove.push_back(i);
            }
        }
        sort(cornersToRemove.begin(), cornersToRemove.end(), std::greater<>());
        for(int ci : cornersToRemove) {
            this->corners.erase(this->corners.begin() + ci);
        }
    }

    return (box.origin[0]!=-1);
}

/* Debug only */
void StackablePallet::printStuff() {
    cout << "\n###### PALLET STATE ######" << endl;
    cout << "placed boxes:" << endl;
    for(box_t box : this->placements) {
        cout << "BOX_"<<box.id<<": origin=("<<box.origin[0]<<", "<<box.origin[1]<< \
                ") dims=("<<box.dims[0]<<", "<<box.dims[1]<<")"<<endl;
    }
    cout << "corners:" << endl;
    for(corner_t c : this->corners) {
        cout << "("<<c.pos[0]<<","<<c.pos[1]<<"), o="<<(int)c.orientation<<(c.isConcave ? " CONCAVE" : "") << endl;
    }
    cout << endl;
    cout << "SKU Backlog:" <<endl;
    skuLL_t *curr = this->volumeOrderedListHead;
    while(curr != NULL) {
        cout << "[id="<<curr->id<<" V="<<(curr->l*curr->w*curr->h)<<" l,w,h=("<<curr->l<<","<<curr->w<<","<<curr->h<<"), amt="<<curr->amt<<"] --> ";
        curr = (skuLL_t*)curr->next;
    }
    cout <<endl;
    cout <<   "##########################\n" << endl;
}
