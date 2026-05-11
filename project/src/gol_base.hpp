#ifndef GOL_BASE_HPP
#define GOL_BASE_HPP

#include "grid.hpp"
#include <algorithm>

class GOL_Base {
protected:
    Grid grid_in;
    Grid grid_out;

    inline unsigned char count_neighbors(int x, int y) const {
        static const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        static const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        unsigned char neighbors = 0;
        for (int i = 0; i < 8; ++i) {
            if (grid_in.at(x + dx[i], y + dy[i]) == 1) neighbors++;
        }
        return neighbors;
    }

public:
    /*
    The constructor
    We add two extra units to each dimension, to act as ghost borders
    that allow us to make the grid periodic without having to worry
    about out of place indexes each time we want to count neighbors
    */
    GOL_Base(int width, int height, bool padding) 
        : grid_in(width + 2, height + 2, padding), grid_out(width + 2, height + 2, padding) {
        grid_in.randomize();
    }

    virtual void run_iteration() = 0;

    const Grid& getGrid() const { return grid_in; }
};

#endif