#include "gol_mult.hpp"

void GOL_Mult::run_iteration() {
    update_ghost_borders();

    #pragma omp parallel for num_threads(n_threads)
    for (int y = 1; y < grid_in.height - 1; ++y) {
        for (int x = 1; x < grid_in.width - 1; ++x) {
            unsigned char neighbors = count_neighbors(x, y);
            unsigned char currentState = grid_in.at(x, y);

            if (currentState == 1) {
                grid_out.at(x, y) = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                grid_out.at(x, y) = (neighbors == 3) ? 1 : 0;
            }
        }
    }

    std::swap(grid_in.data, grid_out.data);
}

void GOL_Mult::update_ghost_borders() {
    int width = grid_in.width;
    int height = grid_in.height;
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for
        for (int x = 0; x < width; ++x) {
            grid_in.at(x, 0) = grid_in.at(x, height - 2);
            grid_in.at(x, height - 1) = grid_in.at(x, 1);
        }

        #pragma omp for
        for (int y = 0; y < height; ++y) {
            grid_in.at(0, y) = grid_in.at(width - 2, y);
            grid_in.at(width - 1, y) = grid_in.at(1, y);
        }
    }
}